import sys
import os

import Core.System as Sys
op=os.path

class stdinstaller:
    #methods to override for a specific installation project:
    def name(self): raise NotImplementedError
    def default_version(self): raise NotImplementedError
    def download_command(self,version): raise NotImplementedError
    def unpack_command(self,version): return None#if unpacking is needed, this cmd must unpack to source/ subdir
    def src_work_subdir(self,version): return ''#after unpacking, is there a subdir to step into before proceeding with configuration?
    def in_source_buildandconfig(self): return True#normally True for autotools, False for CMake
    def configure_command(self,instdir,srcdir,blddir,version,extra_options): raise NotImplementedError
    def validate_version(self,version): raise NotImplementedError
    def build_command(self,nprocs): return 'make -j%i'%nprocs
    def install_command(self,nprocs): return 'make -j%i install'%nprocs
    def allow_local_source(self): return False
    def libdirs_in_installation(self): return []#provide to get fixed install_names on OSX automatically
    def local_source_to_version(self,srcfile_basename): raise NotImplementedError
    def setup_file_contents(self,instdir): raise NotImplementedError
    def prefix_env_var(self): return None#provide if an environment variable XXX points to the installation area
    def unsetup_file_contents(self,instdir): raise NotImplementedError
    def dbg_flags(self): raise NotImplementedError
    def hide_flags(self): return []

    #helper methods which can be used in derived classes when composing flags for the configuration command:
    def _prune_duplicate_flags(self,flags):
        seen = set()
        out=[]
        for f in reversed(flags):
            if not '=' in f:
                out+=[f]
                continue
            name,_=f.split('=',1)
            if name:
                if name not in seen:
                    out+=[f]
                    seen.add(name)
        out.reverse()
        return out
    def add_parser_options(self,parser):
        return
    def get_parser_options(self,opt):
        return
    def parse_cmdline(self):
        n=self.name()
        from optparse import OptionParser#NOTE: The optparse module is deprecated - we should stop using it at some point

        hide_flags = set(self.hide_flags())
        allowed_hide_flags=['--debug','--jobs','--nocleanup','<customoptions>']
        for hf in hide_flags:
            if not hf in allowed_hide_flags:
                raise RuntimeError('implementation error')

        #hide_customopts = ('<customoptions>' in hide_flags)
        parser = OptionParser(usage='%%prog [options]%s'%('' if '<customoptions>' in hide_flags
                                                          else ' [-- <custom options for configuration step>]'),
                              description='Script for making installation of %s easier.'
                              ' After installation, one must source the setup.sh file'
                              ' in the chosen directory in order to use %s.'%(n,n))
        parser.add_option("-d", "--dir",metavar='DIR',
                          type="string", dest="instdir", default=None,
                          help="Install %s in DIR (if DIR contains the string '%%name' it will expand to %s-<version>[_dbg])."%(n,n.lower()))
        if not '--jobs' in hide_flags:
            parser.add_option("-j", "--jobs",metavar="N",
                              type="int", dest="nprocs", default=0,
                              help="Use up to N parallel processes during the build (default: autodetect)")
        if self.allow_local_source():
            parser.add_option("--srcfile",metavar="SRCFILE",
                              type="string", dest="localsrc", default=None,
                              help="Use previously downloaded SRCFILE rather than downloading a new one")
        parser.add_option("-v", "--version",metavar="VERSION",
                          type="string", dest="version", default=None,
                          help="Version to install (default %s)"%self.default_version())
        parser.add_option("--tmpdir",default='',metavar='DIR',type='string',dest="tmpworkdir",
                          help='Set to specify alternative DIR which to use for all temporary build files.')
        if not '--nocleanup' in hide_flags:
            parser.add_option("--nocleanup",default=False,action='store_true',dest="nocleanup",
                              help='Set to prevent cleanup of temporary source and build files')
        if not '--debug' in hide_flags:
            parser.add_option("--debug",default=False,action='store_true',dest="dbginfo",
                              help='Set to build libs with debug info and keep source files'
                              +' around (for optimal usage of debugging tools like gdb and valgrind).')
        self.add_parser_options(parser)
        args,cfgargs=sys.argv[1:],[]
        if '--' in args and not '<customoptions>' in hide_flags:
            cfgargs = args[args.index('--')+1:]
            args = args[0:args.index('--')]
        (opt, args) = parser.parse_args(args)
        if '--debug' in hide_flags: opt.dbginfo=False
        if '--nocleanup' in hide_flags: opt.nocleanup=False
        if '--jobs' in hide_flags: opt.nprocs=1
        if args:
            parser.error("Unknown arguments: '%s'"%("', '".join(args)))
        if opt.dbginfo:
            cfgargs += self.dbg_flags()
        opt.version_set_by_user = (opt.version is not None)
        if not opt.version_set_by_user:
            opt.version = self.default_version()
        if opt.nprocs<0 or opt.nprocs>999:
            parser.error("Number of jobs argument out of range")
        if not self.validate_version(opt.version):
            parser.error("Invalid version format")
        if opt.localsrc:
            opt.localsrc=op.realpath(op.expanduser(opt.localsrc))
            if not op.exists(opt.localsrc):
                parser.error('Specified local source file not found: %s'%opt.localsrc)
            if not opt.version_set_by_user:
                #must autodetect
                v=self.local_source_to_version(op.basename(opt.localsrc))
                if not v or not self.validate_version(v):
                    parser.error('Could not extract version from name of local source (specify using'
                                 +' --version flag instead): %s'%op.basename(opt.localsrc))
                #if opt.version!=v:
                #    parser.error('Version specified does not correspond to detected version of local source file')
                opt.version=v
                print('::: Notice: Guessing from filename that this is version "%s" (if incorrect, set with --version flag)'%v)
        if not opt.instdir:
            parser.error('Please specify destination directory using --dir=DIR')
        if '%name' in opt.instdir:
            opt.instdir=opt.instdir.replace('%name','%s-%s%s'%(n.lower(),opt.version,'_dbg' if opt.dbginfo else ''))
        opt.instdir=op.realpath(op.expanduser(opt.instdir))
        if not Sys.isemptydir(opt.instdir):
            if op.exists(opt.instdir):
                parser.error("Already exists and not an empty directory: %s"%opt.instdir)
            if not op.exists(op.dirname(opt.instdir)):
                parser.error("Not found: %s"%op.dirname(opt.instdir))
            if not op.isdir(op.dirname(opt.instdir)):
                parser.error("Not a directory: %s"%op.dirname(opt.instdir))
        opt.tmpworkdir = op.realpath(op.expanduser(opt.tmpworkdir)) if opt.tmpworkdir else ''
        if opt.tmpworkdir and not Sys.isemptydir(opt.tmpworkdir):
            if op.exists(opt.tmpworkdir):
                parser.error("Already exists and not an empty directory: %s"%opt.tmpworkdir)
            if not op.exists(op.dirname(opt.tmpworkdir)):
                parser.error("Not found: %s"%op.dirname(opt.tmpworkdir))
            if not op.isdir(op.dirname(opt.tmpworkdir)):
                parser.error("Not a directory: %s"%op.dirname(opt.tmpworkdir))
        if ' ' in opt.instdir:
            parser.error('Destination directory path should not contain spaces')
        self.get_parser_options(opt)
        return opt,cfgargs

    def _content_setup_files(self,instdir):
        fluff='# Autogenerated file for %s installation created with %s'%(self.name(),op.basename(sys.argv[0]))
        fluff=['','#'*(len(fluff)+2),fluff+' #','#'*(len(fluff)+2),'']
        prunepath=['function prunepath() {',
                   '    P=$(IFS=:;for p in ${!1:-}; do [[ $p != ${2}* ]] && echo -n ":$p"; done)',
                   '    export $1="${P:1:99999}"',
                   '}','']
        setup_cont=self.setup_file_contents(instdir)
        pev=self.prefix_env_var()
        if pev:
            #if different installation was sourced, call it's unsetup.sh first:
            setup_cont=['if [ "x${%s:-}" != "x" -a -f "${%s:-}"/unsetup.sh ]; then'%(pev,pev),
                        '    . "${%s:-}"/unsetup.sh'%pev,'fi','']+setup_cont
        unsetup_cont=self.unsetup_file_contents(instdir)
        setup_cont = '\n'.join(['#!/usr/bin/env bash']+fluff+setup_cont+[''])
        unsetup_cont = '\n'.join(['#!/usr/bin/env bash']+fluff+prunepath+unsetup_cont+['','unset prunepath',''])
        return setup_cont,unsetup_cont

    def go(self):
        opt,cfgargs = self.parse_cmdline()
        bldtopdir = opt.tmpworkdir if opt.tmpworkdir else op.join(opt.instdir,'TMPBUILD')
        blddir = op.join(bldtopdir,'build')#if needed, such as for CMake
        srcdir = op.join(bldtopdir,'source')
        srcworkdir = op.join(srcdir,self.src_work_subdir(opt.version))
        Sys.mkdir_p(blddir)
        Sys.mkdir_p(bldtopdir)
        Sys.mkdir_p(srcdir)
        Sys.mkdir_p(srcworkdir)
        os.chdir(srcdir)
        if opt.localsrc:
            dlcmd = ['cp','-rp',Sys.quote(opt.localsrc),'.']
        else:
            dlcmd = self.download_command(opt.version)
        dlcmd = Sys.quote_cmd(dlcmd)
        p='::: '
        print("%sFetching source via: %s"%(p,dlcmd))
        ec=Sys.system(dlcmd)
        if ec:
            print("%sERROR: Failed to get source."%p)
            sys.exit(ec)
        upcmd=self.unpack_command(opt.version)
        if upcmd:
            #allow * wildcard in upcmd:
            u=[]
            for _u in upcmd:
                if '*' in _u:
                    import glob
                    g=glob.glob(_u)
                    assert g,"no files selected for unpacking by wildcard"
                    u+=g
                else:
                    u+=[_u]
            upcmd = Sys.quote_cmd(u)
            print("%sUnpacking source via: %s"%(p,upcmd))
            ec=Sys.system(upcmd)
            if ec:
                print("%sERROR: Failed to unpack source."%p)
                sys.exit(ec)
        if opt.dbginfo:
            new_srcworkdir=op.join(opt.instdir,'source')
            import shutil
            shutil.move(srcworkdir,new_srcworkdir)
            srcworkdir=new_srcworkdir
        cfgcmd=self.configure_command(opt.instdir,srcworkdir,blddir,opt.version,cfgargs)
        cfgcmd = Sys.quote_cmd(cfgcmd)
        os.chdir(srcworkdir if self.in_source_buildandconfig() else blddir)
        print("%sConfiguring via: %s"%(p,cfgcmd))
        ec=Sys.system(cfgcmd)
        if ec:
            print("%sERROR: Failure during configuration stage."%p)
            sys.exit(ec)
        if not opt.nprocs:
            import Core.CpuDetect
            opt.nprocs = Core.CpuDetect.auto_njobs()
        print("%sWill use %i processes for compilation and installation."%(p,opt.nprocs))
        bldcmd = self.build_command(opt.nprocs)
        bldcmd = Sys.quote_cmd(bldcmd)
        print("%sBuilding via: %s"%(p,bldcmd))
        ec=Sys.system(bldcmd)
        if ec:
            print("%sERROR: Failure during build stage."%p)
            sys.exit(ec)
        instcmd = self.install_command(opt.nprocs)
        instcmd = Sys.quote_cmd(instcmd)
        print("%sInstalling via: %s"%(p,instcmd))
        ec=Sys.system(instcmd)
        if ec:
            print("%sERROR: Failure during installation stage."%p)
            sys.exit(ec)
        print("%sCreating setup.sh and unsetup.sh for usage of installation"%p)
        sc,usc = self._content_setup_files(opt.instdir)
        try:
            fh=open(op.join(opt.instdir,'setup.sh'),'w')
            fh.write(sc)
            fh.close()
            fh=open(op.join(opt.instdir,'unsetup.sh'),'w')
            fh.write(usc)
            fh.close()
        except Exception as e:
            print("%sERROR: Failure while trying to create setup.sh and unsetup.sh files"%p)
            print("       ",str(e))
            sys.exit(1)
        lds=[os.path.join(opt.instdir,l) for l in self.libdirs_in_installation()]
        lds_to_fix=[l for l in lds if os.path.isdir(l)]
        if lds_to_fix and sys.platform=='darwin':
            print("%sOSX detected - proceeding to fix install_names in installed libraries"%p)
            import DevTools.FixOSXLibInstallNames as FixLib
            for ld in lds_to_fix:
                FixLib.set_install_names_to_abs_path(ld,p)
        print(p)
        print("%s%s successfully installed in:"%(p,self.name()))
        print(p)
        print("%s   %s"%(p,Sys.quote(opt.instdir)))
        print(p)
        print("%sTo use it you can type the command (or put it in $HOME/.bashrc):"%p)
        print(p)
        print("%s   . %s/setup.sh"%(p,Sys.quote(opt.instdir)))
        print(p)
        print("%sTo undo the above, type instead:"%p)
        print(p)
        print("%s   . %s/unsetup.sh"%(p,Sys.quote(opt.instdir)))
        print(p)
        print("%sIf in the future wish to remove the installation, simply remove the entire"%p)
        print("%sdirectory with this command (beware, always double-check rm -rf commands!!):"%p)
        print(p)
        print("%s   rm -rf %s"%(p,Sys.quote(opt.instdir)))
        print(p)
        if not opt.nocleanup:
            Sys.rm_rf(bldtopdir)
        else:
            print("%sAs requested, the build and source files are left in this directory, which"%p)
            print("%sis safe to remove later:"%p)
            print(p)
            print("%s   %s"%(p,Sys.quote(bldtopdir)))
