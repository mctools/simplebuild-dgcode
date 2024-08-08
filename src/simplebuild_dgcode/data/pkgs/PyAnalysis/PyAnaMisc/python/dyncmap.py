__all__=['dynamic_2d_colormap']

import numpy as np
from matplotlib.colors import LinearSegmentedColormap

def dynamic_2d_colormap(data,imgobj=None,ncols=65536):
    #attempt to use high-precision floating point for intermediate calculations:
    try:
        import decimal
        _=decimal.Context(prec=999)
        hpfloat,hpabs,hpmin,hpmax=_.create_decimal,_.abs,_.min,_.max
        hpceil = lambda x : x.to_integral_value(rounding=decimal.ROUND_CEILING)
    except ImportError:
        import math
        hpfloat,hpabs,hpmin,hpmax,hpceil=float,abs,min,max,math.ceil

    hpncols=hpfloat(ncols)
    zero,one=hpfloat(0.0),hpfloat(1.0)
    #figure out limits (remember that all values in data might be masked):
    rawmin = np.min(data)
    rawmax = np.max(data)
    if rawmin is np.ma.masked and rawmax is np.ma.masked:
        rawmin,rawmax = -one, one
    datamin = hpfloat(rawmin)
    datamax = hpfloat(rawmax)
    ddata=datamax-datamin
    if ddata:
        datamin -= hpfloat(0.005)*hpabs(ddata)
        datamax += hpfloat(0.005)*hpabs(ddata)
    else:
        datamin -= hpfloat(0.5);
        datamax += hpfloat(0.5);

    #NB: In principle there is a bug here, ispos+isneg should be based on
    #rawmin/rawmax not datamin/datamax. However, at least the code in
    #NCSABVal/scripts/sabstudio is relying on the present behaviour, so we can't
    #just fix it right away:
    ispos=datamin>=zero
    isneg=datamax<=zero
    #ispos=rawmin>=zero
    #isneg=rawmax<=zero

    if ispos and datamin<zero:
        datamin=zero
    if isneg and datamax>zero:
        datamax=zero
    if not ispos and not isneg:
        #make sure that 0.0 falls on a "bin-edge" in the colormap, so as to
        #not let small positive values accidentally acquire blue values or
        #vice versa:
        assert datamax>zero and datamin<zero
        d0=(datamax-datamin)/hpncols
        m=hpmin(datamax,-datamin)
        k=hpmax(hpceil(m/d0),4)#at least 4 bins on the smallest side
        assert k<hpncols
        eps=(k*(datamax-datamin)-hpncols*m)/(hpncols-k)
        if datamax > -datamin: datamin -= eps
        else: datamax += eps
        # d=(datamax-datamin)/hpncols

    #relative root-mean-square clipped to [0.0001,0.3]:
    npmean=np.mean(np.square(data))
    if npmean is np.ma.masked:
        rms = hpfloat(0.1)
    else:
        rms=hpmax(hpfloat(0.0001),
                  hpmin(hpfloat(0.3),
                        hpfloat(np.sqrt(npmean))/(datamax-datamin)))
    if datamin>=0.0:
        cdict = {'red':   [(0.0,  0.10, 0.10),
                           (float(rms),  0.7, 0.7),
                           (float(3*rms),  1.0, 1.0),
                           (1.0,  1.0, 1.0)],
                 'green': [(0.0,  0.0, 0.0),
                           (1.0,  1.0, 1.0)],
                 'blue':  [(0.0,  0.0, 0.0),
                           (1.0,  0.1, 0.1)]}
    elif datamax<=0.0:
        cdict = {'red':   [(0.0,  0.1, 0.1),
                           (1.0,  0.0, 0.0)],
                 'green': [(0.0,  1.0, 1.0),
                           (1.0,  0.0, 0.0)],
                 'blue':  [(0.0,  1.0, 1.0),
                           (float(one-3*rms),  1.0, 1.0),
                           (float(one-rms),  0.7, 0.7),
                           (1.0,  0.1, 0.1)]}
    else:
        rneg = - datamin / ( datamax - datamin )
        dbin = one/hpncols
        #In rneg-dbin to rneg+dbin, we keep equal mix of red and blue, so
        #numerical errors won't show e.g. very small positive values as blue or
        #vice versa:
        cdict = {'red':   [(0.0,  0.1, 0.1),
                           (float(rneg-dbin),  0.0, 0.1),
                           (float(rneg+dbin),  0.1, 0.1),
                           (float(rneg+hpmax(hpfloat(2)*dbin,(one-rneg)*rms)),  0.7, 0.7),
                           (float(rneg+hpmax(hpfloat(3)*dbin,(one-rneg)*rms*hpfloat(3))),  1.0, 1.0),
                           (1.0,  1.0, 1.0)],
                 'green': [(0.0,  1.0, 1.0),
                           (float(rneg-dbin),  0.0, 0.0),
                           (float(rneg+dbin),  0.0, 0.0),
                           (1.0,  1.0, 1.0)],
                 'blue':  [(0.0,  1.0, 1.0),
                           (float(rneg-hpmax(hpfloat(3)*dbin,rneg*hpfloat(3)*rms)),  1.0, 1.0),
                           (float(rneg-hpmax(hpfloat(2)*dbin,rneg*rms)),  0.7, 0.7),
                           (float(rneg-dbin),  0.1, 0.1),
                           (float(rneg+dbin),  0.1, 0.0),
                           (1.0,  0.1, 0.1)]}
        #for k,v in cdict.items():
        #    print k,v
    import matplotlib.colors
    cmap=LinearSegmentedColormap('dynamic_rbcmap', cdict, N=ncols, gamma=1.0)
    cmap.set_bad('black')#color of masked values (if any)
    if imgobj:
        imgobj.set_clim(float(datamin),float(datamax))
        imgobj.set_cmap(cmap)
    return float(datamin),float(datamax),cmap
