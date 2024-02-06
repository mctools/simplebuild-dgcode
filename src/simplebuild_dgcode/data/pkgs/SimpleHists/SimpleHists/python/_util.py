
def test_and_fix_target(errorfnc,fn):
    import os
    fn=os.path.abspath(os.path.realpath(os.path.expanduser(fn)))
    if os.path.exists(fn):
        errorfnc('Target file %s already exists'%os.path.relpath(fn))
    if not fn.endswith('.shist'):
        fn+='.shist'
    if not os.path.isdir(os.path.dirname(fn)):
        errorfnc('Target directory %s does not exist'%os.path.relpath(os.path.dirname(fn)))
    return fn

def selected_keys(keys,selections):
    """Selections can be either keys (with wildcards) or indices for the key list
(ranges with '-' are supported). Commas inside entries will lead them to be
treated as multiple entries."""
    import string
    import fnmatch
    def contains_alpha(astring):
        return any(c in string.ascii_letters for c in astring)
    def isdigit(astring):
        #this is a non-locale dependent version!
        return astring and all(c in '0123456789' for c in astring)
    sel_pattern=[]
    sel_idx=set()
    sel_ranges=[]
    for s in selections:
        for e in s.split(','):
            if not e:
                raise ValueError("invalid empty selection encountered")
            if contains_alpha(e):
                sel_pattern+=[e]
            else:
                if '-' in e:
                    s=e.split('-')
                    if not len(s)==2 or not isdigit(s[0]) or not isdigit(s[1]):
                        raise ValueError("invalid selection: '%s'"%e)
                    sel_ranges+=[(int(s[0]),int(s[1]))]
                else:
                    if not isdigit(e):
                        raise ValueError("invalid selection: '%s'"%e)
                    sel_idx.add(int(e))
    res=[]
    for i,k in enumerate(keys):
        ok=False
        if i in sel_idx:
            ok=True
        if not ok:
            for ll,u in sel_ranges:
                if ll<=i<=u:
                    ok=True
                    break
        if not ok:
            for s in sel_pattern:
                if fnmatch.fnmatch(k,s):
                    ok=True
                    break
        if ok:
            res+=[(i,k)]
    return res
