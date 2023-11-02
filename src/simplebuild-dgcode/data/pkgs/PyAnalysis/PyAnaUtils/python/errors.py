
def errorbar_args(contents,errors, binedges,style=True):
    """Transform binned data into keyword dict suitable for passing to matplotlib axes.errorbar(..)"""
    from PyAnaUtils.bins import bin_centers_and_widths
    bin_centers,bin_widths = bin_centers_and_widths(binedges)
    kw={'x':bin_centers,
        'y':contents,
        'xerr':bin_widths,
        'yerr':errors}
    if style:
        kw.update({'fmt':'o',#dont connect with line
                   'mec':'black',
                   'mfc':'black',
                   'ecolor':'black',
                   'elinewidth':1 })
    return kw

def plot_errors(contents,errors, binedges,axes=None,style=True,kwargs={}):
    """do a call to axes.errorbar with the passed data reformated.
    If not passed an axes object, it will also create a figure and do plt.show()"""
    import matplotlib.pyplot as plt

    do_quick_plot = bool(axes==None)
    if do_quick_plot:
        fig,axes = plt.subplots()
    from PyAnaUtils.bins import bin_centers_and_widths
    bin_centers,bin_widths = bin_centers_and_widths(binedges)
    args={}
    args.update(errorbar_args(contents,errors, binedges,style=style))
    args.update(kwargs)
    #axes.errorbar(**errorbar_args(contents,errors, binedges))
    axes.errorbar(**args)
    if do_quick_plot:
        plt.show()
