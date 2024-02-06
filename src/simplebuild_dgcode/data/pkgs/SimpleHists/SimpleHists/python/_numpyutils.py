from __future__ import division
__doc__='internal module for adding numpy compatible methods to SimpleHists'

import numpy

try:
    from matplotlib import __version__ as _mplv
except ImportError:
    _mplv = (99,99)#if matplotlib not present, generate most modern arguments
else:
    _mplv = tuple(int(v) for v in _mplv.split('.')[0:2])

def hcounts_bar_args(self,width=1.0,spacing=0.5):
    #not really needing numpy, but here anyway
    left=[]
    height=[]
    width=width
    pos=0.0
    for c in self.counters:
        left+=[pos]
        height+=[c.value]
        pos+=width+spacing
    leftx= 'x' if _mplv>=(2,1) else 'left'
    return {leftx:left,'height':height,'width': width,'align':'edge'}

def hcounts_errorbar_args(self,width=1.0,spacing=0.5,style=True):
    #not really needing numpy, but here anyway
    centers=[]
    height=[]
    errors=[]
    pos=0.5*width
    for c in self.counters:
        centers+=[pos]
        height+=[c.value]
        errors+=[c.error]
        pos+=width+spacing
    d={'x':centers, 'y':height,'xerr':0.5*width, 'yerr':errors,}
    if style:
        d.update({'fmt':'o',#dont connect with line
                  'mec':'black','mfc':'black','ecolor':'black','elinewidth':1 })
    return d

def h1d_errorbar_args(self,style=True):
    c,e = self.contents(),self.errors()
    d={'x':self.bincenters(), 'y':c,'xerr':0.5*self.binwidth, 'yerr':e}
    if style:
        d.update({'fmt':'o',#dont connect with line
                  'mec':'black','mfc':'black','ecolor':'black','elinewidth':1 })
    return d

def h1d_bar_args(self):
    c,b = self.contents(),self.binedges()
    leftx= 'x' if _mplv>=(2,1) else 'left'
    return {leftx:b[:-1],'height':c,'width': self.binwidth,'align':'edge'}

def h1d_contents(self):
    return numpy.frombuffer(self._rawContents(), dtype=float,count=self.nbins)

def h1d_errors(self):
    e2 = numpy.frombuffer(self._rawErrorsSquared(), dtype=float,count=self.nbins)
    return numpy.sqrt(e2)

def h1d_binedges(self):
    return numpy.linspace(self.xmin,self.xmax,self.nbins+1,True)

def h1d_bincenters(self):
    return numpy.linspace(self.xmin+0.5*self.binwidth,self.xmax-0.5*self.binwidth,self.nbins,True)

def h1d_errorband(self,ndev=1.0,clip_for_log=False):
    """return arrays representing the error band curve of the histogram: x,y-ndev*error,y+ndev*error.
       Due to a matplotlib bug it might be necessary to set clip_for_log=True when doing log plots"""
    assert ndev>0
    curvex = numpy.repeat(self.binedges(),2)[1:-1]
    cont=numpy.repeat(self.contents(),2)
    errors=numpy.repeat(self.errors()*ndev,2)
    clower,cupper = cont-errors,cont+errors
    if clip_for_log:
        #epsilon above 0 for log-plots to avoid weird clipping issue
        clower=numpy.maximum(clower,1.0e-199)
        cupper=numpy.maximum(cupper,clower)#
    return curvex,clower,cupper

def h1d_curve(self):
    """return arrays representing the histogram as a curve: x,y"""
    curvex = numpy.repeat(self.binedges(),2)[1:-1]
    curvey = numpy.repeat(self.contents(),2)
    return curvex,curvey

def h1d_histogram(self):
    """Returns hist,bin_edges just like numpy.histogram(values,bin_edges,weights)"""
    return (self.contents(),self.binedges())

def h1d_fill(self,*args):
    #direct filling of single entries is never efficient in python anyway,
    #so we focus here on passing in arrays.
    if len(args) not in (1,2):
        raise ValueError("Hist1D.fill requires 1 or 2 arguments")

    if isinstance(args[0],numpy.ndarray):
        if not args[0].flags['FORC'] or args[0].dtype!=float:
            raise TypeError("Array filling must use numpy float arrays with contiguous memory")
        assert args[0].itemsize==8

        if len(args)==2:
            if not isinstance(args[1],numpy.ndarray) or not args[1].flags['FORC'] or args[1].dtype!=float:
                raise TypeError("Array filling must use numpy float arrays with contiguous memory")
            assert args[1].itemsize==8
        self._rawfillFromBuffer(*[a.data for a in args])
    else:
        #try single entry fill
        self._rawfill(*args)

def h2d_imshow_args(self):
    return {'X':self.contents().T,'extent':self.extent(),'origin':'lower'}

def h2d_bar3d_args(self,cmap=None):
    hist, xedges, yedges = self.histogram2d()
    elements = self.nbinsx*self.nbinsy
    xpos, ypos = numpy.meshgrid(xedges[:-1], yedges[:-1])
    xpos,ypos,zpos = xpos.flatten(),ypos.flatten(),numpy.zeros(elements)
    dx,dy,dz = self.binwidthx * numpy.ones(elements),self.binwidthy * numpy.ones(elements),hist.T.flatten()
    res={'x':xpos, 'y':ypos, 'z':zpos, 'dx':dx, 'dy':dy, 'dz':dz}
    if cmap:
        import matplotlib.colors as colors
        import matplotlib.pyplot as plt
        absz = abs(dz)
        fracs = absz/(absz.max() or 1.0)
        import matplotlib
        if [int(p) for p in matplotlib.__version__.split('.')[0:2]]>=[1,3]:
            norm = colors.Normalize(fracs.min(), fracs.max())
        else:
            norm = colors.normalize(fracs.min(), fracs.max())
        colors = plt.get_cmap(cmap)(norm(fracs))
        res['color']=colors
    return res

def h2d_extent(self):
    return [self.xmin,self.xmax,self.ymin,self.ymax]

def h2d_contents(self):
    c=numpy.frombuffer(self._rawContents(), dtype=float,count=self.nbinsx*self.nbinsy)
    c.shape=(self.nbinsx,self.nbinsy)
    return c

def h2d_xedges(self):
    return numpy.linspace(self.xmin,self.xmax,self.nbinsx+1,True)

def h2d_yedges(self):
    return numpy.linspace(self.ymin,self.ymax,self.nbinsy+1,True)

def h2d_histogram2d(self):
    """Returns hist,xedges,yedges just like numpy.histogram2d(xvalues,yvalues,[xedges,yedges],weights)"""
    return (self.contents(),self.xedges(),self.yedges())

def h2d_fill(self,*args):
    #direct filling of single entries is never efficient in python anyway,
    #so we focus here on passing in arrays.
    if len(args) not in (2,3):
        raise ValueError("Hist2D.fill requires 2 or 3 arguments")
    nnp=0
    for a in args:
        if isinstance(a,numpy.ndarray):
            nnp+=1
            if not a.flags['FORC'] or a.dtype!=float:
                raise TypeError("Array filling must use numpy float arrays with contiguous memory")
                assert a.itemsize==8
    if nnp:
        self._rawfillFromBuffer(*[a.data for a in args])
    else:
        #try scalar entry fill
        self._rawfill(*args)
