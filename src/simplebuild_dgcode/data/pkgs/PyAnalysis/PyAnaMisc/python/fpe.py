def disableFPEDuringCall(obj,fctname):
    import Core.misc
    if not Core.misc.is_debug_build():
        return
    orig = getattr(obj,fctname)
    import Core.FPE
    nofpectxmgr = Core.FPE.DisableFPEContextManager
    def safefct(*args,**kwargs):
        with nofpectxmgr():
            return orig(*args,**kwargs)
    setattr(obj,fctname,safefct)

__standardFixesDone = [False]
def standardMPLFixes():
    global __standardFixesDone
    if __standardFixesDone[0]:
        return
    __standardFixesDone[0] = True
    import Core.misc
    if not Core.misc.is_debug_build():
        return

    try:
        import matplotlib.pyplot
    except ImportError:
        return#do nothing

    import matplotlib.figure
    import matplotlib.scale
    try:
        import matplotlib.backends.backend_pdf
    except ImportError:
        pass
    disableFPEDuringCall(matplotlib.pyplot,'tight_layout')
    disableFPEDuringCall(matplotlib.figure.Figure,'tight_layout')
    disableFPEDuringCall(matplotlib.figure.Figure,'savefig')
    disableFPEDuringCall(matplotlib.pyplot,'show')
    disableFPEDuringCall(matplotlib.pyplot,'plot')
    disableFPEDuringCall(matplotlib.pyplot,'savefig')
    if hasattr(matplotlib.scale,'LogTransformBase'):
        disableFPEDuringCall(matplotlib.scale.LogTransformBase,'transform_non_affine')
    if hasattr(matplotlib.scale,'LogTransform'):
        disableFPEDuringCall(matplotlib.scale.LogTransform,'transform_non_affine')
