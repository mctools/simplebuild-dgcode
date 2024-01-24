import Units
import pathlib
import numpy as np
import copy

def parse(fh):
    metadata={}
    procs={}
    if isinstance(fh,str):
        fh=open(fh)
        assert fh

    if isinstance(fh,pathlib.Path):
        fh=fh.open('rt')
        assert fh

    def storagetrf(x):
        return np.asarray(x)

    currentproc=None
    currentprocdata=[]
    for ll in fh:
        ll=ll.strip()
        if ll.startswith('#'):
            key,val = [e.strip() for e in ll[1:].split(':',1)]
            if not metadata:
                assert [key,val]==['FileType','Geant4 XSECTSPY DATA']
            if key.startswith('Process '):
                assert metadata['EnergyUnit']=='eV'
                assert metadata['XSectUnit']=='barn'
                unit_energy = Units.units.eV
                unit_xsect = Units.units.barn
                if currentproc:
                    procs[currentproc] = storagetrf(currentprocdata)
                currentproc=val
                currentprocdata=[]
                xsect2mfp = float(metadata['ConvertXSectBarnToMeanFreePathMilliMeter'])
                xsect2mfp *= ( Units.units.mm * Units.units.barn )
            else:
                metadata[key]=val
        else:
            assert currentproc is not None
            vals=[float(e) for e in ll.split()]
            assert len(vals)==2
            _xs = vals[1]*unit_xsect
            currentprocdata+=[( vals[0]*unit_energy, _xs, xsect2mfp/_xs )]
    if currentproc:
        procs[currentproc] = storagetrf(currentprocdata)

    return {'metadata':metadata,'procs':procs}


def extract_neutron_xs(xsectfile,collapse_g4procs=True):
    """Process the parsed data, with the ability to collapse neutron cross
    sections into "scatter" (hadElastic) and "absorption" (the rest)"""
    if not isinstance(xsectfile,dict):
        xsectfile = parse(xsectfile)
    md,procs=xsectfile['metadata'],xsectfile['procs']
    assert md['ParticleName']=='neutron'
    assert md['EnergyUnit']=='eV'
    assert md['XSectUnit']=='barn'
    #check all energy grids are the same:
    #pdata_Total = procs['Total']
    procs = sorted([ (pkey,pdata) for (pkey,pdata) in procs.items() if pkey!='Total'])

    def getE(_pdata):
        return _pdata[:,0]

    def getXS(_pdata):
        return _pdata[:,1]

    d = {}
    if collapse_g4procs:
        def createXSFct(_pdata):
            import scipy.interpolate
            return scipy.interpolate.interp1d( getE(_pdata),
                                               getXS(_pdata),
                                               bounds_error=False,
                                               fill_value=0.0,
                                               assume_sorted=True)
        pdata_hadElas = [pdata for (pkey,pdata) in procs if pkey=='hadElastic'][0]
        pdatas_absn = [pdata for (pkey,pdata) in procs if pkey!='hadElastic']

        allE_abs = np.unique(np.concatenate(tuple(getE(pd) for pd in pdatas_absn),0))
        allE_scat = getE(pdata_hadElas)
        allE = np.unique(np.concatenate((allE_abs,allE_scat),0))
        #shave off lowest values to avoid cross sections showing up as having
        while allE[0]<=max(allE_abs[0],allE_scat[0]):
            allE = allE[1:]

        xs_absorption = np.zeros(len(allE))
        for pdata in pdatas_absn:
            xs_absorption += createXSFct(pdata)(allE)
        xs_scatter = createXSFct(pdata_hadElas)(allE)
        d['scatter'] = (allE,xs_scatter)
        d['absorption'] = (allE,xs_absorption)
    else:
        for pkey,pdata in procs:
            d[pkey] = (getE(pdata),getXS(pdata))

    return copy.deepcopy(dict(metadata=md,procs=d))




def create_tgraphs(datafile,unitx=None,unity=None,show_mfp=False,
                   emin=None,emax=None,produce_TotalWoHadElastic=False):
    """Obsolete function, not recommended or supported."""
    import warnings
    warnings.warn('The create_tgraphs function is deprecated')
    import ROOT
    import RootUtils.HistFile

    def apply_style(tgr,col,title,procname):
        dstitle = '%s [%s]'%(title,procname)
        tgr.SetTitle(procname)
        tgr.SetName(dstitle)
        tgr.SetMarkerColor(col)
        tgr.SetMarkerStyle(1)
        tgr.SetMarkerSize(1)
        tgr.SetLineColor(col)
        tgr.SetFillColor(ROOT.kWhite)

    if unitx is None:
        unitx=Units.units.eV
    if unity is None:
        if show_mfp:
            unity=Units.units.cm
        else:
            unity=Units.units.barn

    xsectdata = parse(datafile)
    md=xsectdata['metadata']
    procs=xsectdata['procs']
    title = '%s for %s in material %s'%(('mean-free-path' if show_mfp else 'cross-sections'),md['ParticleName'],md['Material'])
    ges=[]
    cols=[ROOT.kBlue,ROOT.kGreen,ROOT.kOrange,
          ROOT.kBlue+2,ROOT.kGreen+2,ROOT.kOrange+2]
    icol=0
    xvals_all=set()
    nxvals_max=0
    pts_total=None
    #tgr_hadElastic=None
    tgs_nonHadElastic=[]
    for procname,points in procs.items():
        xvals=[]
        yvals=[]
        for i,(energy,xsect,mfp) in enumerate(points):
            if emax is not None and energy>emax:
                continue
            if emin is not None and energy<emin:
                continue
            xvals+=[energy/unitx]
            yvals+= [(mfp if show_mfp else xsect)/unity]

        if not xvals:
            continue
        nxvals_max = max(nxvals_max,len(set(xvals)))
        for xv in xvals:
            xvals_all.add(xv)
        assert len(xvals_all)>=nxvals_max
        ge=RootUtils.HistFile.points2tgraph(xvals,yvals)
        if procname=='Total':
            col=ROOT.kRed
        else:
            col=cols[icol]
        icol = (icol+1)%len(cols)
        apply_style(ge,col,title,procname)
        ges+=[ge]
        if produce_TotalWoHadElastic:
            if procname=='Total':
                pts_total=(xvals,yvals)
            elif procname=='hadElastic':
                pass#tgr_hadElastic=ge
            else:
                tgs_nonHadElastic+=[(xvals,ge)]

    if produce_TotalWoHadElastic:
        yvals=[]
        for i in range(len(pts_total[0])):
            x=pts_total[0][i]
            #x,y=pts_total[0][i],pts_total[1][i]
            #y_hadElastic = tgr_hadElastic.Eval(x)
            ysum=0
            for xvals,tgr in tgs_nonHadElastic:
                if not (x<min(xvals) or x>max(xvals)):
                    ysum += tgr.Eval(x)
            yvals+=[ysum]

        ge=RootUtils.HistFile.points2tgraph(pts_total[0],yvals)
        apply_style(ge,cols[icol],title,'TotalWOhadElastic')
        ges+=[ge]

    return (xsectdata,ges)
