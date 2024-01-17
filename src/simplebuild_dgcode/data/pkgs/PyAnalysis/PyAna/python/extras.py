""" Various common plotting utilities """

#colors inspired by http://www.mulinblog.com/a-color-palette-optimized-for-data-visualization/
#supposedly from Stephen Few’s book, "Show Me the Numbers":
palette_Few = dict(red = "#F15854",
                   blue="#5DA5DA",
                   orange="#FAA43A",
                   green="#60BD68",
                   brown="#B2912F",
                   purple="#B276B2",
                   yellow="#DECF3F",
                   pink="#F17CB0",
                   gray="#4D4D4D")

def axisFormat(axis=None,fmt=None,*,locator=None,locator_minor=None,fmt_minor=None):
    """Helps setting up axis labels and tick positions.

    If axis is not supplied, plt.gca().xaxis is used.

    If fmt is supplied, it is is used as in: axis.set_major_formatter(FormatStrFormatter(fmt))
    Fmt=="deg" or "degree" or "degrees" are special values, meaning fmt=r'$%d^BACKSLASHcirc$'

    The locator is used as in: axis.set_major_locator(MultipleLocator(locator))

    fmt_minor and locator_minor are similar, but affect the minor ticks.

    """
    import matplotlib.pyplot as plt
    axis = axis or plt.gca().xaxis
    _texdeg=r'$%d^\circ$'
    fmt_map = dict(deg=_texdeg,degree=_texdeg,degrees=_texdeg)
    fmt = fmt_map.get(fmt,fmt)
    fmt_minor=fmt_map.get(fmt_minor,fmt_minor)

    if fmt is not None or fmt_minor is not None:
        from matplotlib.ticker import FormatStrFormatter
        if fmt is not None:
            axis.set_major_formatter(FormatStrFormatter(fmt))
        if fmt_minor is not None:
            axis.set_minor_formatter(FormatStrFormatter(fmt_minor))
    if locator is not None or locator_minor is not None:
        from matplotlib.ticker import MultipleLocator
        if locator is not None:
            axis.set_major_locator(MultipleLocator(locator))
        if locator_minor is not None:
            axis.set_minor_locator(MultipleLocator(locator_minor))

def floatToScientificLatex(x):
    """
    Converts number to tex-code, with scientific notation when appropiate (e.g. 1.23e7 becomes "1.23BACKSLASHcdot10^{7}")
    """
    mantissa,exponent=(float(_) for _ in f'{x:e}'.lower().split('e'))
    if mantissa==1.0:
        return '10^{%g}'%exponent
    if not exponent:
        return '%g'%mantissa
    else:
        return '%g%s10^{%g}'%(mantissa,r'\cdot',exponent)

def enable_tex_fonts():
    import matplotlib
    matplotlib.rcParams.update({'text.usetex':1,
                                'font.family':'serif',
                                'font.serif':['cmr10','cm'],
                                'text.latex.preamble' : r'\usepackage{amsmath}'})#LaTeX fonts
def useAGG():
    """Unsets DISPLAY and makes matplotlib use AGG backend"""
    import os
    if 'DISPLAY' in os.environ:
        del os.environ['DISPLAY']
    import matplotlib
    matplotlib.use('agg')

__ask_pdf=[None]
def askPDF():
    """Checks for --pdf on commandline. If found, removes it from sys.argv,
    calls useAGG() and returns True. Otherwise returns False."""
    global __ask_pdf
    import sys
    if '--pdf' in sys.argv[1:]:
        args=sys.argv[1:]
        args.remove('--pdf')
        sys.argv=[sys.argv[0]] + args
        useAGG()
        __ask_pdf[0]=True
        return True
    __ask_pdf[0]=False
    return False


def tnsStyle(pdf='ask'):
    """For easy setting up scripts which creates plots for papers in "TNS" style

    Add the following two lines at the top of your script, before other matplotlib-related imports:

    import PyAna.extras
    PyAna.extras.tnsStyle(pdf=True)

    where pdf=True, if specified, removes the DISPLAY environment variable and
    selects the matplotlib backend "agg". If pdf='ask' (the default), the pdf
    status will be queried with the askPDF function.
    """

    if pdf=='ask':
        pdf = askPDF()
    if pdf:
        useAGG()
    import matplotlib.style
    matplotlib.style.use('classic')
    enable_tex_fonts()

    import matplotlib
    matplotlib.rcParams['xtick.labelsize'] = 12*1.2 #was 'medium' (12pt)
    matplotlib.rcParams['ytick.labelsize'] = 12*1.2 #was 'medium' (12pt)
    matplotlib.rcParams['axes.labelsize']  = 12*1.2 #was 'medium' (12pt)
    matplotlib.rcParams['lines.linewidth'] = 1.5
    matplotlib.rcParams['legend.fontsize'] = 12

    #Remove legend frame (only works with plt.legend, not ax.legend):
    import matplotlib.pyplot as plt
    _plt_legend_orig = plt.legend
    def _plt_legend(*args,**kwargs):
        notouchframelw = False
        if 'notouchframelw' in kwargs:
            notouchframelw = True
            del kwargs['notouchframelw']
        ll = _plt_legend_orig(*args,**kwargs)
        if not notouchframelw:
            ll.get_frame().set_linewidth(0.0)
        return ll
    plt.legend=_plt_legend

def show(filename=None,tight=True,dpi=1200,automode=True):
    """

    Either saves figure (if filename given) or shows it interactively (the latter will use
    PyAna.plt.show2()). If saving, it will use bbox_inches='tight' if tight is True.

    In automode, filename must always be given and user must have called askPDF earlier.

    """
    assert dpi==1200,"dpi argument actually no longer supported"
    from PyAna import plt
    #if tight:
    #    plt.tight_layout()
    if automode:
        if __ask_pdf[0] is None:
            raise RuntimeError('this show function requires usage of askPDF function earlier')
        if not filename:
            raise RuntimeError('this show function requires filename argument when automode=True')
        if not __ask_pdf[0]:
            filename=None

    if filename:
        plt.savefig(filename,bbox_inches=('tight' if tight else None))
        print("Wrote %s"%filename)
    else:
        plt.show2()
    plt.cla()
    plt.clf()
