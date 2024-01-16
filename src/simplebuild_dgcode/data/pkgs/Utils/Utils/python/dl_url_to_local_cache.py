from __future__ import print_function
import Core.System as Sys
import os

def _clean_expr(x):
    import re
    return re.sub('\W|^(?=\d)','_', x)

def dl_with_cache(url,cachedir=None):
    if not '://' in url:
        return url
    op=os.path
    if not cachedir:
        cachedir='/tmp/$USER/dgcode_filecache'
    cachedir=op.expanduser(op.expandvars(cachedir))
    cachedir=os.path.join(cachedir,_clean_expr(url))
    if not op.exists(cachedir):
        Sys.mkdir_p(cachedir)
    target=op.join(cachedir,op.basename(url) or 'content')
    if op.exists(target):
        print('Acquiring file from local cache: %s'%url)
    else:
        import urllib.request
        print('Retrieving remote file to local cache: %s'%url)
        urllib.request.urlretrieve(url, target)

    return target
