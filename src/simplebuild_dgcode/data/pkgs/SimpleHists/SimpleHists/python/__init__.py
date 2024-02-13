__metaclass__ = type#py2 backwards compatibility
__doc__='python module for package SimpleHists'
__all__=['Hist1D','Hist2D','HistBase','HistCollection',
         'histTypeOfData','deserialise']

###############################################################################
# 1) Include hist classes etc. from the compiled C++ module:
from . _init import *

from os import environ as _environ

###############################################################################
# 2) Detect presence of numpy and if present, we add methods for getting
#    contents/errors as numpy arrays as well as a .histogram[2d] method which
#    returns contents in a format similar to that returned by
#    numpy.histogram(..) and numpy.histogram2d(..). We also enable efficient
#    filling with numpy arrays.

try:
    import numpy
except ImportError:
    numpy=None

if numpy:
    import SimpleHists._numpyutils as nu
    Hist1D.bar_args = nu.h1d_bar_args
    Hist1D.errorbar_args = nu.h1d_errorbar_args
    Hist1D.contents = nu.h1d_contents
    Hist1D.errors = nu.h1d_errors
    Hist1D.binedges = nu.h1d_binedges
    Hist1D.bincenters = nu.h1d_bincenters
    Hist1D.curve = nu.h1d_curve
    Hist1D.errorband = nu.h1d_errorband
    Hist1D.histogram = nu.h1d_histogram
    Hist1D.fill = nu.h1d_fill
    Hist2D.contents = nu.h2d_contents
    Hist2D.xedges = nu.h2d_xedges
    Hist2D.yedges = nu.h2d_yedges
    Hist2D.extent = nu.h2d_extent
    Hist2D.imshow_args = nu.h2d_imshow_args
    Hist2D.bar3d_args = nu.h2d_bar3d_args
    Hist2D.histogram2d = nu.h2d_histogram2d
    Hist2D.fill = nu.h2d_fill
    HistCounts.bar_args = nu.hcounts_bar_args
    HistCounts.errorbar_args = nu.hcounts_errorbar_args
else:
    Hist1D.fill=Hist1D._rawfill
    Hist2D.fill=Hist2D._rawfill

################################################################################
# 3) Detect presence of matplotlib and if present, we add methods for quickly
#    plotting the histograms in interactive viewers.

matplotlib=None
if numpy:
    try:
        import matplotlib
    except ImportError:
        pass



def is_osx_catalina_or_later():
    import platform
    if platform.system()!='Darwin':
        return False
    return int(platform.release().split('.')[0])>=19

_lacks_display_var = not _environ.get('DISPLAY',None)
_catalina_mode = is_osx_catalina_or_later()
_has_agg_backend = (matplotlib and matplotlib.get_backend().lower()=='agg')
if (not _has_agg_backend) and _lacks_display_var and _catalina_mode:
    #insert dummy value to prevent matplotlib from switching to the agg backend:
    _environ['DISPLAY']=':0'
    _lacks_display_var=False
_noninteractive = _has_agg_backend or _lacks_display_var
if 'SIMPLEHISTS_SILENT_PLOTFAIL' in _environ:
    _noninteractive = True

if (not _noninteractive) and matplotlib:
    import SimpleHists.plotutils as plotutils
    Hist1D.plot = plotutils.plot1d
    Hist1D.overlay = plotutils.plot1d_overlay
    Hist2D.plot = plotutils.plot2d
    Hist2D.plot_lego = plotutils.plot2d_lego
    HistCounts.plot = plotutils.plotcounts
else:
    def no_plot(self,*args,**kwargs):
        if 'SIMPLEHISTS_SILENT_PLOTFAIL' in _environ:
            return
        if _noninteractive:
            reason=('running in non-interactive mode (agg backend'
                    ' selected or empty DISPLAY variable can cause this)')
        else:
            reason='please install numpy and matplotlib'
        raise SystemExit(f"ERROR: Plotting not available ({reason})")
    Hist1D.plot = no_plot
    Hist1D.overlay = no_plot
    Hist2D.plot = no_plot
    Hist2D.plot_lego = no_plot
    HistCounts.plot = no_plot

###############################################################################
# 4) Make histograms accessible as HistCollection properties so they can be
#    easily accessed via tab completion in ipython. Note that this only works
#    nicely for keys of the form (alpha)+(alpha/digit/_)*

class _histgetter:
    def __init__(self,hc):
        self._hc=hc
    def __dir__(self):
        return dir(self.__class__)+self._hc.keys
    def __getattr__(self,k):
        if k[0]=='_':
            return super(_countgetter,self).__getattr__(k)
        return self._hc.hist(k)
    def __iter__(self):
        for k in self._hc.keys:
            yield self._hc.hist(k)

HistCollection.hist_getter = property(lambda self: _histgetter(self))

###############################################################################
# 5) Support fspath interface and thus allow pathlib.Path and similar objects
#    when specifying file locations:

__HistCollection_init_orig = HistCollection.__init__
__HistCollection_saveToFile_orig = HistCollection.saveToFile
def __HistCollection_init(self,*args,**kwargs):
    if args and hasattr(args[0],'__fspath__'):
        args = (args[0].__fspath__(),)+args[1:]
    return __HistCollection_init_orig(self,*args,**kwargs)
def __HistCollection_saveToFile(self,*args,**kwargs):
    if args and hasattr(args[0],'__fspath__'):
        args = (args[0].__fspath__(),)+args[1:]
    return __HistCollection_saveToFile_orig(self,*args,**kwargs)
HistCollection.__init__ = __HistCollection_init
HistCollection.saveToFile = __HistCollection_saveToFile

###############################################################################
# 6) Make counts accessible as HistCounts properties so they can be easily
#    accessed via tab completion in ipython.

class _countgetter:
    def __init__(self,hc):
        self._hc=hc
    def __dir__(self):
        res=dir(self.__class__)
        for c in self._hc.counters:
            res+=[c.label]
        return res
    def __getattr__(self,k):
        if k[0]=='_':
            return super(_countgetter,self).__getattr__(k)
        return self._hc.getCounter(k)

HistCounts.c = property(lambda self: _countgetter(self))

