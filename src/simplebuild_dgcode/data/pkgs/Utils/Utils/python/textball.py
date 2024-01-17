"""Provide support for "textballs". A simple alternative to tar-balls, which is
   better suited for being committed to a git repo, in the sense that modifying
   text files inside these "textballs" will result in small diff's. It is also
   easy to open an editor and edit text files inside them directly. Any binary
   (non-utf8) files will be included after base64 encoding.  Binary files are
   supported but will be ascii85-encoded.

"""

#Possible improvements:
#  * Work on input streams and files in addition to strings.
#  * Have TextBallWriter class for writing.
#  * Support storage of empty directories. Issue is that such empty directories
#    will need special treatment when iterating over files and returning
#    (path,content) pairs.
#  * support links
#  * Work recursively. I.e. textballs should be able to contain textballs. This will
#    require the magic_common string to be escaped in text data!
#    Perhaps: #~~~~~~~~~~> -> @@TEXT_BALL_ESCAPE@@[#~~~~~~~~~~>#~~~~~~~~~~>]

import Utils.PathUtils as PU
import base64
import pathlib
# import stat

class _TextBallDefs:
    magic_common='\n\n#~~~'#NOTE: '~' is neither used in base64 or ascii85 encodings!
    magic_linetext=magic_common+"textfile~~~>>> "
    magic_linebinary=magic_common+"binaryfile~~~>>> "
    magic_executable='~executable~ '
    hdr='@TEXTBALL_v1@\n'

class TextBallIter:

    """Use to open textball files and iterate through the contents, in the form of
       Utils.PathUtils.VirtualFile objects. These objects provide the usual
       information for VirtualFile objects, with the path being the relative
       file path specified inside the textball.
    """

    #NB: careful everywhere not to create needless copies of potentially huge
    #input data, so in several places we are using data.find(..) with specific
    #indices, rather than the more usual data[i:j]

    def _fmterr(self):
        raise RuntimeError('Invalid input to textball_unpack: Data not in right format')

    def __init__(self,data):
        if hasattr(data,'__fspath__'):
            data = pathlib.Path(data).read_bytes().decode('utf8')#like this, not .read_text(..) to preserve dos/unix newlines
        if not isinstance(data,str) or not data.startswith(_TextBallDefs.hdr):
            self._fmterr()
        self._data = data
        self._nextsearchstart = len(_TextBallDefs.hdr)
        self._nextsection = self._find_next_section()

    def __iter__(self):
        d = self._data
        while True:
            if self._nextsection is None:
                break
            _,idx_filename,is_binary_this = self._nextsection
            self._nextsection = self._find_next_section()
            if self._nextsection is None:
                idx_hdr_next = len(d)
            else:
                idx_hdr_next,_,_ = self._nextsection
            idx_fn_end = d.find('\n\n',idx_filename)
            if idx_fn_end == -1:
                self._fmterr()
            idx_content = idx_fn_end + 2#skip '\n\n'
            if idx_content >= idx_hdr_next:
                self._fmterr()
            filename = d[idx_filename:idx_fn_end]
            if filename.startswith(_TextBallDefs.magic_executable):
                filename = filename[len(_TextBallDefs.magic_executable):]
                is_exe = True
            else:
                is_exe = False
            contents = d[idx_content:idx_hdr_next]
            if is_binary_this:
                contents=base64.a85decode(contents)
            is_dir=False#Not supported yet
            yield PU.VirtualFile( path = pathlib.Path(filename),
                                  content = contents,
                                  is_exe = is_exe,
                                  is_dir = is_dir )

    def _find_next_section(self):
        """Returns (index_header_begin,index_filename_begin,is_binary), or None in case of no more sections."""
        d = self._data
        no_more_sections = None
        if d is None:
            return no_more_sections
        assert self._nextsearchstart is not None
        isearchnext = self._nextsearchstart
        while True:
            i = d.find(_TextBallDefs.magic_common,isearchnext)
            if i == -1:
                return no_more_sections
            itext = d.find(_TextBallDefs.magic_linetext,i)
            if itext==i:
                #yep, is text:
                self._nextsearchstart = itext + len(_TextBallDefs.magic_linetext)
                return itext,self._nextsearchstart,False
            ibinary = d.find(_TextBallDefs.magic_linebinary,i)
            if itext==-1 and ibinary==-1:
                return no_more_sections
            if itext!=-1 and ibinary!=-1:
                if itext < ibinary:
                    ibinary=-1
                else:
                    itext=-1
            if itext==-1:
                #is binary:
                self._nextsearchstart = ibinary + len(_TextBallDefs.magic_linebinary)
                return ibinary,self._nextsearchstart,True
            else:
                #is text:
                self._nextsearchstart = itext + len(_TextBallDefs.magic_linetext)
                return itext,self._nextsearchstart,False

def filter_exclude_spurious_files( path ):
    #Catch emacs / vi backup files, as well as any file with '#'/'~', or a few other spurious files.
    #*~
    #\#*\#
    #.\#*
    #.*.sw?
    n=path.name
    if '~' in n or '#' in n:
        return False
    if n.startswith('.') and n[-4:].startswith('.sw'):
        return False
    if n == '.DS_Store' or n.startswith('vgcore.'):
        return False
    return True


_normpath = lambda p : pathlib.Path(p).absolute().resolve()

def textball_create( topdir,
                     target = None,
                     quiet = False,
                     path_filter = filter_exclude_spurious_files,
                     include_empty_dirs = True,
                     mark_executable_files = True ):
    """Extract content of all files below topdir into a single "text ball" which is
       returned as a string (unless target is provided in which case it is
       written). This is similar to a tar-ball, but text files inside can be
       directly read when inspecting the file, can simply be edited in-place
       with a text editor if desired, and small changes provide small intuitive
       diff's. Binary files (defined as those not valid UTF8) are supported as
       well, but are ascii64-encoded. Thus, the entire returned string can be
       encoded in UTF8 format.

    """
    _print = (lambda *a, **kwa : None) if quiet else print
    topdir = _normpath(topdir)
    assert topdir.exists()
    out = _TextBallDefs.hdr
    if not path_filter:
        path_filter = lambda p : True

    for f in sorted(topdir.rglob('*')):
        if f.is_dir():
            continue
        relpath = f.relative_to(topdir)
        relpath_str = str(f.relative_to(topdir))
        if not relpath == pathlib.Path(relpath_str):
            raise RuntimeError(f'Could not encode<->decode relative path as string: {relpath}')
        try:
            relpath_str.encode('utf8')
        except UnicodeEncodeError:
            raise RuntimeError(f'File name can not be encoded in UTF8: {relpath_str}')
        if '\n' in relpath_str:
            raise RuntimeError(f'New line characters not allowed in file name: {relpath}')
        if not path_filter(f):
            _print(f'textball_create: ignoring {relpath_str}')
            continue
        _print(f'textball_create: processing {relpath_str}')

        content_bytes = f.read_bytes()
        try:
            content_text = content_bytes.decode('utf8')
            content_bytes = None
        except UnicodeDecodeError:
            content_text = None#binary data
        filename_for_file = ( _TextBallDefs.magic_executable if PU.is_usr_executable(f) else '') + relpath_str + '\n\n'
        if content_text is not None:
            #First check against magic line (we could - but have not - implemented an escaping scheme):
            for ml in [_TextBallDefs.magic_linetext,_TextBallDefs.magic_linebinary]:
                if ml in content_text:
                    raise RuntimeError(f'ERROR: Magic line "{ml}" encountered in text!!!')
            out += _TextBallDefs.magic_linetext
            out += filename_for_file
            out += content_text
        else:
            #binary
            out += _TextBallDefs.magic_linebinary
            out += filename_for_file
            out += base64.a85encode(content_bytes,wrapcol=80).decode('ascii')
    if target:
        _print(f'textball_create: writing {target}')
        PU.write_file_preserve_newlines(_normpath(target),out)
    return out

def textball_unpack( textball,
                     target_dir,
                     parents = False,
                     exist_ok = False,
                     quiet = False,
                     path_filter = None ):
    td = _normpath(target_dir)
    if td.exists() and not td.is_dir():
        raise RuntimeError(f'Can not unpack text ball in non-directory: {td}')
    td.mkdir(parents=parents,exist_ok=exist_ok)
    for subpath,content in TextBallIter(textball):
        dest=td / subpath
        if dest.exists():
            raise RuntimeError(f'Error while unpacking - file already exists: {dest}')
        dest.parent.mkdir(parents=True,exist_ok=True)
        PU.write_file_preserve_newlines(dest,content)

#if __name__=='__main__':
#    import os
#    fn_textball=pathlib.Path('./dummy.textball')
#    fn_unpackdir = pathlib.Path('./dummy_unpackdir')
#    for _ in [fn_textball,fn_unpackdir]:
#        if _.exists():
#            raise SystemExit(f'ERROR: Please remove and rerun: {_}')
#    print("====>>> TEST: Creating text ball")
#    tbstr=textball_create(pathlib.Path(/some/where/todo) / 'packages/Examples/Skeletons/SkeletonSP' )
#    print(f"====>>> TEST: Writing {fn_textball.name}")
#    _write_file_preserve_newlines(fn_textball,tbstr)
#    print(f"====>>> TEST: Unpacking to {fn_unpackdir.name}")
#    textball_unpack(fn_textball,fn_unpackdir)

