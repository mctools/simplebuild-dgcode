
def decode_inputfile(fn, require_ext=None, rel_path = False ):
    import os.path as op
    def _fix(f):
        f=op.abspath(op.expanduser(f))
        if require_ext and not fn.endswith(require_ext):
            return False,'does not have extension %s'%require_ext
        return True,(op.relpath(f) if rel_path else f)
    if '~' in fn or op.exists(fn):
        return _fix(fn)
    parts = fn.split('/')
    if len(parts)==2 and parts[0] and parts[1]:
        #could be PKGNAME/FILENAME specification
        import Core.FindData3
        tfn = Core.FindData3(*parts)
        if tfn and op.exists(tfn):
            return _fix(tfn)
    return False,'does not exists'

def decode_outputfile(fn, require_ext=None, rel_path = False, can_exist=False):
    import os.path as op
    fn=op.abspath(op.expanduser(fn))
    if not fn:
        return False,'is empty path'
    if op.isdir(fn):
        return False,'is a directory'
    if not can_exist and op.exists(fn):
        return False,'already exists'
    if not op.isdir(op.dirname(fn)):
        return False,'is in non-existing directory'
    if require_ext and not fn.endswith(require_ext):
        return False,'does not have extension %s'%require_ext
    if rel_path:
        fn=op.relpath(fn)
    return True,fn
