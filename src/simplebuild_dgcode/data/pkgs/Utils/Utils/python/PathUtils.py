import pathlib
import stat

def normpath(path,strict_resolve=False):
    return pathlib.Path(path).expanduser().absolute().resolve(strict=strict_resolve)

def is_empty_dir(path):
    return path.is_dir() and not any(path.iterdir())

def make_executable(path):
    #For each lvl (usr/grp/other) with a read bit set, also set the executable bit.
    m = path.stat().st_mode
    m |= (m & 292) >> 2
    path.chmod( m )

def is_usr_executable(path):
    return bool( path.stat().st_mode & stat.S_IXUSR )

def write_file_preserve_newlines(path,content):
    """Write content to path. If content is raw bytes, they will be written
       directly. Otherwise, they will be UTF8 encoded and written (thus
       preserving all newlines as they are in the input!)
    """
    path.write_bytes(content if isinstance(content,bytes) else content.encode('utf8'))

class VirtualFile:
    """Virtual file object with a path (typically relative) and content (typically
       bytes if binary, str if text), and flags representing executable state and
       directories (for directories content must be None and executable is always False)"""
    def __init__( self, path, content, *, is_exe = False, is_dir = False ):
        if is_dir:
            assert content is None and is_exe is False
        self.__p = path
        self.__c = content
        self.__x = is_exe
        self.__d = is_dir

    def modified(self,**kwargs):
        if not 'path' in kwargs:
            kwargs['path'] = self.__p
        if not 'content' in kwargs:
            kwargs['content'] = self.__c
        if not 'is_exe' in kwargs:
            kwargs['is_exe'] = self.__x
        if not 'is_dir' in kwargs:
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
        write_file_preserve_newlines(dest,self.__c)
        if self.__x:
            make_executable(dest)

def write_vfiles_under_dir(vfile_iter, targetbasedir, *, write_callbackfct = None ):
    #returns number of files written
    td = normpath( targetbasedir )
    if td.exists():
        if not td.is_dir():
            raise RuntimeError(f'Error: Target directory exists and is not a file: {td}')
        if not is_empty_dir(td):
            raise RuntimeError(f'Error: Target directory not empty: {td}')
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

