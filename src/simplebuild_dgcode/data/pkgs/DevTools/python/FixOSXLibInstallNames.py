#!/usr/bin/env python3

def set_install_names_to_abs_path(libdir,prefix='',verbose=False):
    import os,sys

    if sys.platform!='darwin':
        print("%sSystem not OSX - will not do anything"%prefix)
        return#ignore on linux

    import glob
    from Core.System import system_throw,quote

    libdir=os.path.abspath(os.path.realpath(os.path.expanduser(libdir)))

    # In any lib whose install name is equal to its basename (or the basename of
    # any symlinks in the same directory), we want to change the install name to
    # it's absolute path name.
    #
    # In addition, to not break inter-dependencies, we must also fix dependent
    # install names to the lib from any sister libs in the same directory.
    #
    # The reason for having to do this is that OSX 10.11 (El Capitan) by default
    # does not allow usage of variables such as DYLD_LIBRARY_PATH in scripts run
    # via interpreters in system dirs (including '#!/usr/bin/env python3' and
    # #!/usr/bin/env bash).

    #First build up a list of all files in the directory (and their symlink aliases):
    class Lib:
        def __init__(self,basename):
            self.basename=basename
            self.symlinks=[]
            self.install_name_orig=None
            self.install_name_new=None
            self.deps=[]#install names of deps whose install name will change
        def install_name_changed(self):
            assert self.install_name_orig is not None
            assert self.install_name_new is not None
            return self.install_name_orig != self.install_name_new

    # libs=[]
    rbn2lib={}#basename of real lib files -> Lib objects
    #abn2lib={}#basename of both sym links and real libs -> Lib objects

    #First build up rbn2lib without install_name and dep infos:
    files  = glob.glob(os.path.join(libdir,'*.dylib'))
    files += glob.glob(os.path.join(libdir,'*.so'))
    for file in files:
        bn=os.path.basename(file)
        if os.path.islink(file):
            rbn = os.path.relpath(os.path.realpath(file),libdir)
            if os.path.sep in rbn:
                print("ignoring non-local symlink:",file)
            #assert not os.path.sep in rbn,"lib symlink goes out of specified libdir!"
            if not rbn in rbn2lib:
                rbn2lib[rbn] = Lib(rbn)
            rbn2lib[rbn].symlinks += [bn]
        else:
            rbn = os.path.basename(file)
            if not rbn in rbn2lib:
                rbn2lib[rbn] = Lib(rbn)
        #assert not bn in abn2lib
        #abn2lib[bn] = rbn2lib[rbn]

    #Next use otool to figure out install names and how they should be changed:
    instname_map={}
    for rbn,lib in sorted(rbn2lib.items()):
        assert lib.basename==rbn
        absname=os.path.join(libdir,rbn)
        assert os.path.exists(absname) and not os.path.islink(absname)
        tmp = system_throw("otool -D %s"%quote(absname),True)
        lib.install_name_orig = tmp.split('\n')[1].strip()
        if not os.path.sep in lib.install_name_orig:
            lib.install_name_new = os.path.join(libdir,lib.install_name_orig)
        elif lib.install_name_orig.startswith('@rpath/'):
            lib.install_name_new = os.path.join(libdir,lib.install_name_orig[7:])
        else:
            lib.install_name_new = lib.install_name_orig#unchanged - already absolute path?
        if lib.install_name_changed():
            if lib.install_name_orig in instname_map:
                if lib.install_name_new==instname_map[lib.install_name_orig]:
                    continue#already ok?
                print("ERROR: Multiple libs has install name",lib.install_name_orig)
                print("Both",lib.install_name_new,'and',instname_map[lib.install_name_orig])
            assert not lib.install_name_orig in instname_map
            instname_map[lib.install_name_orig] = lib.install_name_new

    #Next use otool to figure out dependencies:
    for rbn,lib in sorted(rbn2lib.items()):
        absname=os.path.join(libdir,rbn)
        tmp = system_throw("otool -L %s"%quote(absname),True).split('\n')
        assert tmp[0].strip().startswith(absname)
        for l in tmp[1:]:
            l=l.strip().split()
            if not l: continue
            depinstname=l[0]
            if depinstname in instname_map:
                lib.deps += [ depinstname ]#dont put objects here to avoid circular
                                           #deps (even though they would be odd)

    #Time for action!
    for rbn,lib in sorted(rbn2lib.items()):
        nfix=0
        afn=os.path.join(libdir,rbn)
        if verbose:
            print("%sTreating library %s:"%(prefix,quote(afn)))
        if not lib.install_name_changed() and not lib.deps:
            if verbose:
                print("%sNothing to do"%prefix)
            else:
                print('%sChanged 0 install names in %s'%(prefix,quote(afn)))
            continue
        if lib.install_name_changed():
            cmd='install_name_tool -id %s %s'%(quote(lib.install_name_new),quote(afn))
            if verbose:
                print("%s   %s"%(prefix,cmd))
            nfix+=1
            system_throw(cmd)
        for d in lib.deps:
            cmd='install_name_tool -change %s %s %s'%(quote(d),quote(instname_map[d]),quote(afn))
            if verbose:
                print("%s   %s"%(prefix,cmd))
            nfix+=1
            system_throw(cmd)
        if not verbose:
                print('%sChanged %i install names in %s'%(prefix,nfix,quote(afn)))


if __name__=='__main__':
    import sys
    set_install_names_to_abs_path(sys.argv[1])
