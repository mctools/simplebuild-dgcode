###################################################################
# Some hacks to fix plots for TNS:

import sys,matplotlib

increase_fontsize = not ('deffont' in sys.argv[1:])
remove_legendbox = not ('showlegbox' in sys.argv[1:])
units_softbrackets = not ('squnitbrackets' in sys.argv[1:])
increase_linewidth = not ('deflw' in sys.argv[1:])

if increase_fontsize:
    matplotlib.rcParams['xtick.labelsize'] = 12*1.2 #was 'medium' (12pt)
    matplotlib.rcParams['ytick.labelsize'] = 12*1.2 #was 'medium' (12pt)
    matplotlib.rcParams['axes.labelsize'] = 12*1.4 #was 'medium' (12pt)

if increase_linewidth:
    matplotlib.rcParams['lines.linewidth'] = 2

from PyAna import *

if remove_legendbox:
    _plt_legend_orig = plt.legend
    def _plt_legend(*args,**kwargs):
        notouchframelw = False
        if 'notouchframelw' in kwargs:
            notouchframelw = True
            del kwargs['notouchframelw']
        l = _plt_legend_orig(*args,**kwargs)
        if not notouchframelw:
            l.get_frame().set_linewidth(0.0)
        return l
    plt.legend=_plt_legend
