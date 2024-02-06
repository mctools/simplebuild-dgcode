"""Quickly overlay contents of a series of histograms while optionally
   rebinning, normalising, etc.

"""

__author__="thomas.kittelmann@ess.eu"

def copyhist(hist,rebin=None,normalise=False):
    h=hist.clone()
    if rebin is not None:
        h.rebin(rebin)
    if normalise:
        h.norm()
        if normalise=='%':
            h.scale(100.0)
    return h

__ihist = 0
__html=[]
def cmphists(histograms_in,
             rebin=None,
             normalise='%',
             **kwargs):
    """Overlay plot of passed in histograms. histograms_in is a list
    [(legendkey,histogram)]. Rebin and normalise arguments are used to control
    rebinning and normalisation. All other arguments are passed along to
    SimpleHists.Hist1D.overlay

    """
    global __ihist
    global __html
    __ihist+=1
    assert histograms_in

    def modhist(h):
        if (rebin is not None and rebin!=h.nbins) or normalise:
            h=h.clone()
            if (rebin is not None and rebin!=h.nbins):
                h.rebin(rebin)
            if normalise:
                h.norm()
                if normalise=='%':
                    h.scale(100.0)
        return h
    keys  = [k for k,h in histograms_in]
    hists = [modhist(h) for k,h in histograms_in]
    return hists[0].overlay(hists[1:],keys,**kwargs)
