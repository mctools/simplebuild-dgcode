
__all__ = ['math','os','sys','pathlib','numpy', 'matplotlib', 'np', 'mpl', 'plt','Units','units','ColourDashesHelper','lighten_colour','darken_colour','rainbow_colour','dgextras']

#1) Fix matplotlib backend:
import os
import matplotlib
if os.getenv("DISPLAY"):
    #possibly interactive, use backend tkagg unless gtkagg is chosen
    _backend = matplotlib.get_backend().lower()
    if _backend=='gtkagg':
        import PyAna._fix_backend_gtk#quench some warnings
    elif _backend!='tkagg':
        matplotlib.use('tkagg')
else:
    #not interactive, choose agg
    matplotlib.use('agg')

#2) Make sure the standard analysis modules are available, including standard aliases:
import PyAna.extras as dgextras
import math
#import os (already imported above)
import sys
import pathlib
import numpy
# import warnings
# try:
#     import scipy
# except ImportError:
#     warnings.warn('scipy not available')
#     scipy=None
#import matplotlib (already imported above)
import matplotlib.pyplot
np = numpy
# sp = scipy
mpl = matplotlib
plt = matplotlib.pyplot
import Units as _Units
Units = _Units
units = _Units.units

#2b) Add plt.show2 as plt.show with extra improvements (not replacing plt.show
#directly to avoid messing with existing users who might not want the
#enhancements):
plt.show2 = plt.show
if matplotlib.get_backend().lower()=='tkagg':
    #fixme, we could support other backends as well:
    #http://www.blog.pythonlibrary.org/2015/08/18/getting-your-screen-resolution-with-python/
    _auto_dpi = []
    def _show2(*args,**kwargs):
        import matplotlib.pyplot as plt
        if len(_auto_dpi)==0:
            _dpi = int(plt.get_current_fig_manager().window.winfo_screenwidth()*0.06)
            _auto_dpi.append(_dpi if _dpi>max(120.0,float(matplotlib.rcParams['figure.dpi'])) else None)
        if _auto_dpi[0] is not None:
            plt.gcf().set_dpi(_auto_dpi[0])
        def kh(evt):
            if evt and evt.key and evt.key.lower()=='ctrl+a':
                print('CTRL-A detected - aborting execution.')
                raise SystemExit
        _cnv=plt.gcf().canvas
        _cnv.mpl_connect('key_press_event',kh)
        rv=plt.show(*args,**kwargs)
        _cnv.mpl_disconnect(kh)
        return rv
    plt.show2 = _show2

#Make Qt auto-detect screen scale (should not hurt when using other backends):
#if not "QT_AUTO_SCREEN_SCALE_FACTOR" in os.environ:
#    os.environ["QT_AUTO_SCREEN_SCALE_FACTOR"] = "1"


#3) disable FPE's certain matplotlib function calls (we don't care too
#   much about FPE's in other peoples gui/plotting code):
from PyAnaMisc.fpe import standardMPLFixes as _standardMPLFixes
_standardMPLFixes()

#4) Make all legends draggable by default (unless new kw notdraggable=True) and set default numpoints=1 :
_plt_legend_orig = plt.legend
def _plt_legend(*args,**kwargs):
    if not 'numpoints' in kwargs:
        kwargs['numpoints']=1
    l = _plt_legend_orig(*args,**kwargs)
    if not 'notdraggable' in kwargs:
        if hasattr(l,'set_draggable'):
            l.set_draggable(True)
        elif hasattr(l,'draggable'):
            l.draggable()
    else:
        del kwargs['notdraggable']
    return l
plt.legend=_plt_legend
#Make sure help(plt.legend) still look nice:
plt.legend.__doc__='[NB: Overridden version with numpoints=1 and draggable unless notdraggable=True]\n\n'+_plt_legend_orig.__doc__
plt.legend.__name__=_plt_legend_orig.__name__
plt.legend.__module__=_plt_legend_orig.__module__

#5) Make viewers quit-able with 'q' or 'Q':
if 'keymap.quit' in plt.rcParams and not 'q' in plt.rcParams['keymap.quit']:
    plt.rcParams['keymap.quit'] = tuple(list(plt.rcParams['keymap.quit'])+['q','Q'])

#6) convenience function:
plt.enable_tex_fonts = dgextras.enable_tex_fonts

_cm_spectral = plt.cm.spectral if hasattr(plt.cm,'spectral') else plt.cm.nipy_spectral
def rainbow_colour(i,n):
    assert i<n
    cmap = lambda x : _cm_spectral(0.07+x*(0.9-0.07))
    return cmap(i/(n-1.0) if n>1 else 1.0)

class ColourDashesHelper:
    def __init__(self,ncurves,dashes=[(),(5,2),(2,2)],cmap=None):
        num_styles = len(dashes)
        if not cmap:
            from matplotlib.pyplot import get_cmap
            cmap = get_cmap('gist_rainbow')
        num_colours = ncurves//num_styles + (1 if ncurves%num_styles else 0)
        colors=[cmap(1.*i/num_colours) for i in range(num_colours)]
        from itertools import product
        self.styles=list(reversed(list(product(colors,reversed(dashes)))))[0:ncurves]
    def pop(self):
        color,dashes = self.styles.pop(0)
        return color,dashes
    def pop_kwargs(self):
        color,dashes = self.styles.pop(0)
        return dict(color=color,dashes=dashes)

def lighten_colour(col, amount=0.5):
    """
    Lightens the given colour by multiplying (1-luminosity) by the given amount.
    Input can be matplotlib colour string, hex string, or RGB tuple.

    Examples:
    >> lighten_colour('g', 0.3)
    >> lighten_colour('#F034A3', 0.6)
    >> lighten_colour((.3,.55,.1), 0.5)
    """
    #https://stackoverflow.com/questions/37765197/darken-or-lighten-a-color-in-matplotlib
    import matplotlib.colors as mc
    import colorsys
    try:
        c = mc.cnames[col]
    except:
        c = col
    c = colorsys.rgb_to_hls(*mc.to_rgb(c))
    return colorsys.hls_to_rgb(c[0], max(0, min(1, c[1]/amount)), c[2])

def darken_colour(col, amount=0.5):
    return lighten_colour(col,1.0/amount)
