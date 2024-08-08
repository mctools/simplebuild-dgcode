import matplotlib

#we want to move from gtkagg to tkagg, but if gtkagg is present we allow it (since it works - at least for now).
_backend = matplotlib.get_backend().lower()
_backend_ok = (_backend in ('tkagg','gtkagg','agg','macosx'))

if _backend_ok and _backend=='gtkagg':
    #attempt to silence some useless/confusing warnings
    try:
        import PyAnaMisc._fix_backend_gtk # noqa F401
    except ImportError:
        #perhaps PyAnaMisc package was not built, no biggie, users might get warnings.
        pass

if _backend_ok and _backend=='tkagg':
    if [float(n) for n in matplotlib.__version__.split('.',2)[:-1]]>=[1,5]:
        #hide broken (warnings https://github.com/matplotlib/matplotlib/issues/7404/):
        import warnings
        warnings.filterwarnings("ignore",
                                category=UserWarning,
                                message=('Treat the new Tool classes introduced in v1.5 as experimental'+
                                         ' for now, the API will likely change in version 2.1'))

from PyAnaMisc.fpe import standardMPLFixes as _standardMPLFixes # noqa E402
_standardMPLFixes()

def _ensure_backend_ok():
    if _backend_ok:
        return
    import os
    import sys
    if hasattr(matplotlib,'get_configdir'):
        _cfgdir=matplotlib.get_configdir()
        _cfgdir=_cfgdir.replace(os.path.expanduser('~'),'~')
    else:
        if [float(n) for n in matplotlib.__version__.split('.',2)[:-1]]>=[1,3]:
            _cfgdir = '~/.config/matplotlib'
        else:
            _cfgdir='~/.matplotlib'
    _cfgdir_filler=' '*(30-len(_cfgdir))
    print("""
    ############################################################################################################
    ############################################################################################################
    ###                                                                                                      ###
    ###  ERROR:                                                                                              ###
    ###                                                                                                      ###
    ###         SimpleHists plotting utilities require the platform specific matplotlib                      ###
    ###         drawing backend to be "tkagg", but it is "%s".%s           ###
    ###                                                                                                      ###
    ###  HOW TO FIX YOUR SYSTEM:                                                                             ###
    ###                                                                                                      ###
    ###         First test that the tkagg backend works for you by running                                   ###
    ###                                                                                                      ###
    ###          python3 -c 'import matplotlib as m;m.use("tkagg");import matplotlib.pyplot;print("all ok")' ###
    ###                                                                                                      ###
    ###         If you get "all ok", change the default backend by running the two commands:                 ###
    ###                                                                                                      ###
    ###           mkdir -p %s%s                                                    ###
    ###           echo "backend : tkagg" >> %s/matplotlibrc%s                      ###
    ###                                                                                                      ###
    ###  TROUBLESHOOTING:                                                                                    ###
    ###                                                                                                      ###
    ###         If the test for tkagg does not give "all ok", then you might have to reinstall matplotlib    ###
    ###         after installing tcl/tk headers. Alternatively you can try the "gtkagg" backend which        ###
    ###         requires gtk and pygtk to be installed. On MacOS you might want to use "macosx"              ###
    ###                                                                                                      ###
    ############################################################################################################
    ############################################################################################################
    """%(_backend,max(0,(38-len(_backend)))*' ',_cfgdir,_cfgdir_filler,_cfgdir,_cfgdir_filler))
    sys.exit(1)
