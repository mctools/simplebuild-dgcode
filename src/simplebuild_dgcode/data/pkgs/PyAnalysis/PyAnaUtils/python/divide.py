"""module for performing division of sets of values including errors (such as histograms)"""
from __future__ import division

import numpy

def divide(values1,errors1,values2,errors2):
    """Divide two sets of values and associated errors to produce a new set of values and propagated errors"""
    values = values1/values2
    errors = abs(values)*numpy.sqrt((errors1/values1)**2+(errors2/values2)**2)
    return values,errors

def divide_histograms(h1,h2):
    """Extracts data from two SimpleHists, divides them, and returns the result including propagated errors"""
    errors1=h1.errors()
    errors2=h2.errors()
    contents1=h1.contents()
    contents2=h2.contents()
    bins=h1.binedges()
    bins2=h2.binedges()
    assert numpy.array_equal(bins,bins2)#not so efficient, but could catch stupid bugs.
    return divide(contents1,errors1,contents2,errors2)

def plot_histogram_division(h1,h2,scale=1.0,axes=None,style=True,kwargs={}):
    """In addition to performing histogram division, it also invokes errors.plot_errors on the result"""
    vals,errs = divide_histograms(h1,h2)
    from PyAnaUtils.errors import plot_errors
    plot_errors(vals*scale,errs*scale,h1.binedges(),axes=axes,style=style,kwargs=kwargs)
