"""ExportMgr provides an easy and consistent way to export some of our code for
external users. All I/O should happen via the ExportMgr, so we can modify
sources and monitor which files we depend on (for extraction of git commit id).
"""
import Core.System as Sys
import Utils.GitUtils
import os,sys,shutil,datetime
import simplebuild.cfg as cfg

import pathlib
AbsPath = lambda p : pathlib.Path(p).expanduser().resolve().absolute()

class ExportMgr:
    def __init__(self,projectname,args=None):
        """Init object. Must always supply a projectname. If args are not supplied, sys.argv[1:] is used instead."""
        self.projectname = projectname
        self.__files_read = set()
        self.__files_readwgetlines = set()
        self.mark_file_as_used(__file__)
        self.__files_written = set()
        self.__files_written_lowercase = set()
        self.__src_versions =[]
        self.pkgdirs = [str(d) for d in cfg.dirs.pkgsearchpath]

        if args is None:
            args = sys.argv[1:]
            progname=os.path.basename(sys.argv[0])
        else:
            progname='<cmd>'

        if any(s in args for s in ('help','--help','-h')):
            print("Usage:\n")
            print("%s [-h|--help] [--skipversioncheck] [--skiptar] [--force] [OUTPUTDIR]\n"%progname)
            sys.exit(0)

        self.skip_git=False
        if '--skipversioncheck' in args:
            args.remove('--skipversioncheck')
            self.skip_git=True
        force=False
        if '--force' in args:
            args.remove('--force')
            force=True
        self.skip_tar=False
        if '--skiptar' in args:
            args.remove('--skiptar')
            self.skip_tar=True
        if len(args)==0:
            dgdir = cfg.dirs.main_bundle_pkg_root
            self.outdir = self._normalisepath(dgdir,'%s_exported'%projectname)
        elif len(args)==1:
            self.outdir = self._normalisepath(args[0])
        else:
            self.error("too many arguments")
        if os.path.exists(self.outdir):
            if force:
                shutil.rmtree(self.outdir)
            else:
                self.error("Remove directory before running: %s"%self.outdir)
        self.tarname = projectname+'-%%VERSION%%'

    def error(self,msg):
        """overwrite for custom error handling (must never return to calling code)"""
        print("ERROR: %s"%msg)
        sys.exit(1)

    def _normalisepath(self,*path):
        p=os.path.abspath(os.path.realpath(os.path.join(*path)))
        if not os.path.islink(p):
            return p
        #realpath apparently does not always resolve links completely...:
        i=0
        while os.path.islink(p):
            i+=1
            p=os.path.readlink(p)
            if i==50:
                self.error('could not resolve link: %s'%p)
        return self._normalisepath(p)

    def packages(self,filter_fct = lambda pkgname,pkgabspath,pkgrelpath: True):
        """Iterator which returns dgcode package names for packages selected by the
        provided filter function."""
        for pkgname,pkg in cfg.pkgs.items():
            if filter_fct(pkgname,pkg['dirname'],pkg['reldirname']):
                yield pkgname

    def dgfile(self,path):
        """Returns full path to file in dgcode. Supports package-centric arguments such
        as "PkgName/libinc/SomeFile.hh" or "PkgName/somedata.dat"."""

        np = self._normalisepath
        if path in cfg.pkgs:
            return np(cfg.pkgs[path]['dirname'])
        for d in self.pkgdirs:
            f=os.path.join(d,path)
            if os.path.exists(f):
                return np(f)
            if path.count('/')==1:
                #Perhaps a data file?
                f=os.path.join(os.getenv('SBLD_DATA_DIR'),path)
                if os.path.exists(f):
                    return np(f)
            if path.count('/') in (1,2):
                #File in pkg, like: PkgName/libsrc/somefile.cc or PkgName/libsrc/
                pkgname,subpath=path.split('/',1)
                if pkgname in cfg.pkgs:
                    f=os.path.join(cfg.pkgs[pkgname]['dirname'],subpath)
                    if os.path.exists(f):
                        return np(f)
        self.error('File not found: %s'%path)

    def mark_file_as_used(self,filename):
        """Manually mark file as a dependency for version monitoring via git
        history."""
        assert filename
        f=self.__fixsrc(filename)
        if f.endswith('.pyo') or f.endswith('.pyc'):
            f=f[:-1]
            if not os.path.exists(f):
                self.error('Could not find .py for .pyc/.pyo: %s'%filename)
        self.__files_read.add(f)

    def provide_lines(self,filename):
        """reimplement this method to filter/edit things like include statements or license blurbs"""
        for l in open(filename):
            yield l

    def getlines(self,filename):
        """Iterate through lines in file (will have been edited by provide_lines if
        reimplemented)."""
        f = self.__fixsrc(filename)
        self.__files_read.add(f)
        self.__files_readwgetlines.add(f)
        makefile = filename.startswith('Makefile') or filename.endswith('.make')
        for i,l in enumerate(self.provide_lines(f)):
            try:
                if isinstance(l,bytes):
                    l.decode('ascii')#py2
                else:
                    l.encode('ascii')#py3
            except (UnicodeDecodeError,UnicodeEncodeError):
                self.error("Output based on file %s is not pure ASCII in line %i (>>>> %s <<<<)"%(f,i+1,l.strip()))
            if '\r' in l:
                self.error("Non-unix line endings encountered in file %s (line %i)"%(f,i+1))
            if '\t' in l and (not makefile or '\t' in l[1:]):
                #tabs only allowed in makefiles at start of lines
                self.error("TABs encountered in file %s (line %i)"%(f,i+1))
            yield l

    def verify_output_file_name(self,filename):
        """reimplement to enforce custom file naming policies"""
        pass

    def __add_file_written(self,f):
        fo=f[len(self.outdir)+1:]
        try:
            if isinstance(fo,bytes):
                fo.decode('ascii')#py2
            else:
                fo.encode('ascii')#py3
        except UnicodeDecodeError:
            self.error("Output filename %s is not pure ASCII"%(f))
        forbidden=set(fo).intersection(set('#~!@$ '))
        if forbidden:
            self.error("forbidden characters in file name: >>%s<< in file %s"%(''.join(forbidden),f))
        fl=f.lower()
        if fl in self.__files_written_lowercase:
            self.error("multiple output files with same lowercase name (gives problems on OSX): %s"%fl)
        self.verify_output_file_name(fo)
        self.__files_written.add(f)
        self.__files_written_lowercase.add(fl)

    def write(self,dest,content,mode='w',make_executable=False):
        """Write content to destination file."""
        dest = self.__fixdest(dest)
        if dest in self.__files_written:
            self.error("multiple writes to same file: %s"%dest)
        self.__add_file_written(dest)
        with open(dest,mode) as fh:
            fh.write(content)
        if make_executable:
            Sys.chmod_x(dest)

    def __fixsrc(self,src):
        if not src.startswith('/'):
            src=self.dgfile(src)
        return self._normalisepath(src)

    def __fixdest(self,dest):
        if not dest.startswith('/'):
            dest=os.path.join(self.outdir,dest)
        dest = self._normalisepath(dest)
        if not dest.startswith(self.outdir):
            self.error('destination outside output directory attempted! (%s is outside %s)'%(dest,self.outdir))
        targetdir = os.path.dirname(dest)
        if not targetdir.startswith(self.outdir):
            self.error('target directory is outside output directory attempted '+
                        '(this is a bit odd...)! (%s is outside %s)'%(targetdir,self.outdir))
        if os.path.exists(targetdir) and not os.path.isdir(targetdir):
            self.error('Can not make dir where file is already: %s'%targetdir)
        if not os.path.exists(targetdir):
            Sys.mkdir_p(targetdir)
        return dest

    def outfile(self,path_in_outdir,must_exist=True):
        """Transform relative path in output directory to absolute system path. Results
        in an error if file does not exist unless must_exist=False."""
        p=self._normalisepath(self.outdir,path_in_outdir)
        if must_exist and not os.path.exists(p):
            self.error('Requested path in output directory does not exist: %s'%path_in_outdir)
        return p

    def copy(self,src,dest,make_executable=False):
        """Copy source file to destination - checking for T-O-D-Os and F-I-X-M-Es and
        performing custom edits on the way if provide_lines is reimplemented"""
        src = self.__fixsrc(src)
        #The write call fixes dest and adds it to __files_written
        self.write(dest,''.join(self.getlines(src)),make_executable=make_executable)

    def copy_unaltered(self,src,dest,make_executable=False):
        """Simply copies the source file directly to the destination"""
        src = self.__fixsrc(src)
        dest = self.__fixdest(dest)
        self.__add_file_written(dest)
        self.__files_read.add(src)
        shutil.copyfile(src,dest)
        if make_executable:
            Sys.chmod_x(dest)

    def __known_versions(self,filename):
        out={}
        for l in open(filename,'r'):
            l=l.strip()
            if not l or l.startswith('#'):
                continue
            commithash,version=l.split('#',1)[0].split()
            #assert len(commithash) == 40
            assert len(commithash) >= 39
            version_parts=tuple(int(v) for v in version.split('.'))
            assert len(version_parts)==3
            assert not commithash in out,'duplicate commit hashes'
            out[commithash]=version_parts
        assert len(set(out.values()))==len(out),'duplicate versions'
        return out

    def _search_t_o_d_o_and_f_i_x_m_e_ignores(self):
        return []

    def __search_t_o_d_o_and_f_i_x_m_e(self):
        dt,df={},{}
        search_bad = { ('to'+'do') : dt, ('fix'+'me') : df}
        ignores = self._search_t_o_d_o_and_f_i_x_m_e_ignores()
        for f in self.__files_read:
            if not f in self.__files_readwgetlines:
                try:
                    fh=open(f,'rt')
                    for _ in fh:
                        break
                except UnicodeDecodeError:
                    continue#not a text file - dont check
            for l in (self.getlines(f) if f in self.__files_readwgetlines else open(f)):
                l=l.lower()
                for i in ignores:
                    l=l.replace(i,' ? ')
                for pat,d in search_bad.items():
                    if pat in l:
                        d.setdefault(f,0)
                        d[f]+=1
        return dt,df

    def found_versions_in_source(self,source_name,version_tuple):
        """Use this method if release versions can be extracted from the code itself
        (like in C/C++ defines, python __version__, etc.). When calling
        finalise_export, this will then be compared to the version deduced (if any)
        for the code via git and the version_file)"""
        for s,v in self.__src_versions:
            if v!=version_tuple:
                self.error("mismatch between versions in %s and %s"%(s,source_name))
        self.__src_versions += [(source_name,version_tuple)]

    def finalise_export(self,version_file,additional_changelog_file=None):
        """Finalise the export procedure by calling this function."""

        for f in ('CHANGELOG','FILES','INSTALL','LICENSE','README'):
            if not self.__fixdest(f) in self.__files_written:
                self.error('Output is missing recommended file : "%s"\n'%f)

        if not self.__src_versions:
            print("WARNING: No versions were extracted from the code and added via found_versions_in_source(..).")

        bad_t,bad_f = self.__search_t_o_d_o_and_f_i_x_m_e()
        n_t = sum(c for f,c in bad_t.items())
        n_f = sum(c for f,c in bad_f.items())
        if n_t:
            print(("WARNING: %i TO"+"DOs found in parsed files:")%n_t)
            for f,c in sorted(bad_t.items()):
                print(' %10i %s'%(c,f))
        else:
            print("No TO"+"DOs found in parsed files!!")
        if n_f:
            print(("WARNING: %i FIX"+"MEs found in parsed files:")%n_f)
            for f,c in sorted(bad_f.items()):
                print(' %10i %s'%(c,f))
        else:
            print("No FIX"+"MEs found in parsed files!!")


        print("Created %i files, now querying git for version info (takes a few seconds)."%len(self.__files_written))

        official_version=None
        def extract_version_sha(topdir,files_quoted):
            with Sys.changedir(topdir):
                _ = Sys.system_throw(' '.join(['TZ=UTC ','git','log',Sys.quote('--pretty=format:%aI /AND/ %H'),
                                               '--max-count=1']+files_quoted),True).decode().split('/AND/')
                assert len(_)==2
                return _[0].strip(),_[1].strip()

        if self.skip_git:
            print("WARNING: requested to skip git version check!!")
            version_string='VERSION-INFO-SKIPPED / CUSTOM-DEV-VERSION'
        else:
            files_read = [self._normalisepath(f) for f in self.__files_read if
                          any([self._normalisepath(f).startswith(str(d)) for d in self.pkgdirs])]
            read_are_dirty = False
            topdirs_and_files_quoted_for_export_sha = []
            for topdir in cfg.dirs.pkgsearchpath:
                files_read_below_topdir = [ AbsPath(f) for f in files_read if AbsPath(topdir) in AbsPath(f).parents ]
                #ignore ourselves:
                #files_read_below_topdir = [f for f in files_read_below_topdir if not f.parts[-3:]==('ExportUtils','python','__init__.py') ]
                files_quoted_below_topdir = [Sys.quote(f) for f in files_read_below_topdir]
                if not files_quoted_below_topdir:
                    print(f"Ignoring commit id of repo not participating in export: {topdir}")
                    continue
                #ignore_for_export_sha = not files_read_below_topdir
                with Sys.changedir(topdir):
                    if list(Utils.GitUtils.git_status_iter(files_read_below_topdir)):
                        read_are_dirty = True
                        print("WARNING: Some of the %i files used below %s are uncommitted"%(len(files_read_below_topdir),
                                                                                             topdir))
                #if not ignore_for_export_sha:
                topdirs_and_files_quoted_for_export_sha.append( (topdir,files_quoted_below_topdir) )

            if read_are_dirty:
                print('WARNING: Not all used files are committed and unmodified!\n')
                d=datetime.datetime.utcnow().isoformat()
                d=d.split('.')[0]
                version_string='%s / CUSTOM-DEV-VERSION'%d
            else:
                #assert len(topdirs_and_files_quoted_for_export_sha)==1
                if not topdirs_and_files_quoted_for_export_sha:
                    raise SystemExit('Unexpected error: no topdirs/files exported!')
                _=list(extract_version_sha(*f) for f in topdirs_and_files_quoted_for_export_sha)
                timestamp=max(*list(sorted(timestamp for timestamp,hashval in _)))
                combinedhash = "%".join(sorted(hashval for timestamp,hashval in _))
                version_string = f'{timestamp} / {combinedhash}'

        if not 'CUSTOM-DEV-VERSION' in version_string:
            official_version=self.__known_versions(self.__fixsrc(version_file)).get(version_string.split('/')[1].strip(),None)

        if official_version:
            version_string = '.'.join(str(i) for i in official_version)
            print()
            print('Detected an official version via %s! : %s\n'%(version_file,version_string))
            print()
            for n,v in self.__src_versions:
                if v!=official_version:
                    self.error("mismatch between versions in %s and %s"%(n,version_file))

            #this should be documented in changelog (always in the internal one, release
            #candidates not necessarily in exported one):
            s='v%s '%version_string
            for i,clfile in enumerate([self.__fixdest('CHANGELOG')]+[additional_changelog_file] if additional_changelog_file else []):
                if i!=0:
                    clfile=self.__fixsrc(clfile)
                nclrefs_int = sum(int(l.startswith(s)) for l in open(clfile))
                if nclrefs_int!=1:
                    s='Exported version (%s) not referenced exactly once in changelog (%s)'%(version_string,clfile)
                    if i>0:
                        self.error(s)
                    else:
                        print('WARNING: %s'%s)
                        print("WARNING: This might be OK only if this is an internal test release not intended for public circulation.")
                        print()
        else:
            print('WARNING: This is an unofficial version, not suitable for export to github!\n')
        self.write(os.path.join(self.outdir,'VERSION'),version_string.strip()+'\n')

        print("Created export in   : %s"%self.outdir)
        print("Version             : %s"%version_string)

        if self.skip_tar:
            print("WARNING: requested to skip creation of release tar-ball!!")
        else:
            version_safe = version_string.replace(' / ','-').replace(':','').lower()
            tarname = self.tarname.replace('%%VERSION%%',version_safe)
            outdir_fortar=self._normalisepath(self.outdir,'..',tarname)+'/'
            if os.path.exists(outdir_fortar):
                self.error("%s already exists!"%outdir_fortar)
            shutil.copytree(self.outdir,outdir_fortar)
            Sys.system_throw('cd %s && cd .. && tar czf %s.tar.gz %s/'%(Sys.quote(outdir_fortar),
                                                                        Sys.quote(tarname),Sys.quote(tarname)))
            shutil.rmtree(outdir_fortar)
            print("Created tar-ball in : %s.tar.gz"%tarname)
        return official_version,version_string
