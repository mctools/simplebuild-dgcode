import SimpleHists
import numpy

def commul_curve(h1,relative=True,above=True,counting_errors=False):
    """Return x,y arrays where y_i is the entries above or below x_i,
    either as an absolute or relative number. Naturally the x_i are
    the bin edges of the passed in histogram"""
    if not isinstance(h1,SimpleHists.Hist1D):
        raise TypeError("Only accepts SimpleHists.Hist1D objects")
    # under=h1.underflow
    # over=h1.overflow
    x=h1.binedges()
    y=numpy.zeros(len(x))
    y[0] = h1.underflow
    y[1:] = numpy.cumsum(h1.contents())
    if above:
        y = h1.integral - y
    if relative:
        y /= h1.integral
    return x,y
