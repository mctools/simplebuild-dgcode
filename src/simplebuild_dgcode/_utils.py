import pathlib

__all__ = []

def normpath(path,strict_resolve=False):
    return ( pathlib.Path(path).expanduser().
             absolute().resolve(strict=strict_resolve) )

def normpath_or_none( p ):
    return normpath(p) if p else None

def is_empty_dir(path):
    return path.is_dir() and not any(path.iterdir())

def make_executable(path):
    #For each lvl (usr/grp/other) with a read bit set,
    #also set the executable bit.
    m = path.stat().st_mode
    m |= (m & 292) >> 2
    path.chmod( m )

def is_usr_executable(path):
    import stat
    return bool( path.stat().st_mode & stat.S_IXUSR )

def recursive_vfile_iter( skeleton_dir ):
    skeldir = pathlib.Path(__file__).parent / 'data' / 'pkgs_val' / 'SkeletonSP'
    assert skeldir.is_dir()
    assert ( skeldir / 'G4GeoSkeletonSP' / 'pkg.info' ).is_file()
    def vfile_iter():
        for p in sorted(skeldir.rglob('*')):
            if p.is_dir():
                continue
            if '~' in p.name or '#' in p.name:
                continue
            yield path_to_vfile( p, p.relative_to( skeldir ) )
    return vfile_iter()

def write_file_preserve_newlines(path,content):
    """Write content to path. If content is raw bytes, they will be written
       directly. Otherwise, they will be UTF8 encoded and written (thus
       preserving all newlines as they are in the input!)
    """
    path.write_bytes( content if isinstance(content,bytes) else content.encode('utf8'))

class VirtualFile:
    """Virtual file object with a path (typically relative) and content
       (typically bytes if binary, str if text), and flags representing
       executable state and directories (for directories content must be None
       and executable is always False)

    """
    def __init__( self, path, content, *, is_exe = False, is_dir = False ):
        if is_dir:
            assert content is None and is_exe is False
        else:
            assert content is not None
        self.__p = path
        self.__c = content
        self.__x = is_exe
        self.__d = is_dir

    def modified(self,**kwargs):
        if 'path' not in kwargs:
            kwargs['path'] = self.__p
        if 'content' not in kwargs:
            kwargs['content'] = self.__c
        if 'is_exe' not in kwargs:
            kwargs['is_exe'] = self.__x
        if 'is_dir' not in kwargs:
            kwargs['is_dir'] = self.__d
        return VirtualFile( **kwargs )

    @property
    def path(self): return self.__p
    @property
    def content(self): return self.__c
    @property
    def executable(self): return self.__x
    @property
    def is_dir(self): return self.__d

    def writeToDestination(self, dest):
        assert not dest.exists()
        dest.parent.mkdir(parents=True,exist_ok=True)
        if self.is_dir:
            dest.mkdir()
        else:
            write_file_preserve_newlines(dest,self.__c)
            if self.executable:
                make_executable(dest)

def path_to_vfile( srcpath, use_path = None ):
    encode_path = srcpath if use_path is None else use_path
    if srcpath.is_dir():
        return VirtualFile( encode_path, content = None, is_dir = True )
    assert srcpath.is_file()

    content_bytes = srcpath.read_bytes()
    try:
        content_text = content_bytes.decode('utf8')
        content_bytes = None
    except UnicodeDecodeError:
        content_text = None#binary data
    content = ( content_text
                if content_bytes is None
                else content_bytes )
    assert content is not None
    is_exe = is_usr_executable( srcpath )
    return VirtualFile( path = encode_path,
                        content = content,
                        is_exe = is_exe )

def write_vfiles_under_dir(vfile_iter,
                           targetbasedir, *,
                           write_callbackfct = None ):
    #returns number of files written
    td = normpath( targetbasedir )
    if td.exists():
        if not td.is_dir():
            raise RuntimeError('Error: Target directory exists'
                               f' and is not a file: {td}')
        if not is_empty_dir(td):
            raise RuntimeError('Error: Target directory'
                               f' not empty: {td}')
    else:
        td.mkdir(parents=True)
    assert td.exists() and td.is_dir() and is_empty_dir(td)
    n = 0
    for vfile in vfile_iter:
        dest = td / vfile.path
        if write_callbackfct:
            write_callbackfct( dest )
        vfile.writeToDestination( dest )
        n += 1
    return n
