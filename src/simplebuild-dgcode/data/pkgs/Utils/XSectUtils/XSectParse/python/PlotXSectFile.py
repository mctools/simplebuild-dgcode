import XSectParse.ParseXSectFile
from Utils.NeutronMath import neutron_eV_to_angstrom
from PyAna import *

def _find_nearest(array,value):
    idx = (np.abs(array-value)).argmin()
    return idx

def _extract_vals(npa,wl,mfp):
    energy_ev=npa[:,0]/Units.units.eV
    yvals = npa[:,2]/Units.units.cm if mfp else npa[:,1]/Units.units.barn
    if wl:
        #convert energy to wl, reverse array direction and limit to 0.1..25Aa
        y=yvals[::-1]
        try:
            fv = np.vectorize(neutron_eV_to_angstrom)
        except ValueError:
            fv = None
        if fv:
            x=fv(energy_ev[::-1])
        else:
            x=np.zeros(len(energy_ev))
            for i,e in enumerate(energy_ev[::-1]):
                x[i] = neutron_eV_to_angstrom(e)
        imin,imax = _find_nearest(x,0.1),_find_nearest(x,25.0)
        return x[imin:imax],y[imin:imax]
    else:
        return energy_ev,yvals

def _plot_begin():
    from Core.System import which
    if which('latex'):
        plt.enable_tex_fonts()
    plt.cla()
    plt.clf()

def _plot_end(show,save_fig,wl,mfp,logx,logy,extra=None,mfpunit='cm',softbrackets=False):
    if extra:
        extra()
    xl = r'Neutron wavelength [\AA]' if wl else r'Energy [eV]'
    yl = ('Mean free path [%s]'%mfpunit) if mfp else r'$\sigma$ [barn]'
    if softbrackets:
        xl = xl.replace('[','(').replace(']',')')
        yl = yl.replace('[','(').replace(']',')')
    plt.xlabel(xl)
    plt.ylabel(yl)
    if logy=='auto':
        if (not wl) or mfp:
            plt.semilogy()
    elif logy:
        plt.semilogy()
    if logx=='auto':
        if not wl:
            plt.semilogx()
    elif logx:
        plt.semilogx()
    if wl:
        from matplotlib.ticker import MultipleLocator, FormatStrFormatter
        plt.gca().xaxis.set_major_locator(MultipleLocator(5))
        plt.gca().xaxis.set_major_formatter(FormatStrFormatter('%d'))
        plt.gca().xaxis.set_minor_locator(MultipleLocator(1))
    plt.legend()
    plt.grid()
    try:
        plt.tight_layout()
    except (AttributeError, ValueError):
        print("ERROR: tight_layout failed!")
        plt.subplots_adjust(bottom=0.15, right=0.9, top=0.9, left = 0.15)
    if save_fig:
        plt.savefig(save_fig)
    if (show==None and not save_fig) or show:
        plt.show()

def plot_file(filename,mfp=False,save_fig=None,show=None,versus_wavelength=False,logx='auto',logy='auto',softbrackets=False):
    """Plots cross-sections found in file. Set mfp=True to plot mean free path
    rather than cross-sections and set save_fig to save output. The parameter
    'show' controls display of interactive window (default is to show when no
    saving a figure)"""

    _plot_begin()
    showMFP=mfp

    colors_nored = ['c', 'm', 'y','k','g','b']
    p=XSectParse.ParseXSectFile.parse(filename)
    md=p['metadata']
    procs=p['procs']
    physlist = md.get('PhysicsList',None)#not present in older files
    title=r'%s %s in \texttt{%s} (%s g/cm$^{3}$, %s K, %s bar)'%(md['ParticleName'].capitalize(),
                                                            ('MFP' if showMFP else 'X-sections'),
                                                            md['Material'].replace('_',r'\_'),
                                                            md['MaterialDensity [g/cm3]'],
                                                            md['MaterialTemperature [K]'],
                                                            md['MaterialPressure [bar]'])
    #move Total to the end:
    procnames=list(procs.keys())
    if 'Total' in procnames:
        procnames.remove('Total')
        procnames+=['Total']

    for iprocname,procname in enumerate(procnames):
        npa=numpy.asarray(procs[procname])
        if not len(npa):#could be an empty physics list
            continue
        xvals,yvals = _extract_vals(npa,versus_wavelength,showMFP)
        if not len(yvals):
            continue#perhaps we are showing wavelengths and the PL has no contrib in 0.1..25 Aa
        col='r' if procname=='Total' else colors_nored[iprocname%len(colors_nored)]
        plt.plot(xvals,yvals,
                 label=procname,color=col)

    plt.title(title)

    if physlist:
        plt.suptitle(r'\texttt{Geant4} with physics list \texttt{%s}'%physlist.replace('_','\_'))

    _plot_end(show,save_fig,versus_wavelength,showMFP,logx,logy,softbrackets=softbrackets)

def plot_file_cmp(filenames,mfp=False,save_fig=None,show=None,
                  xsectname='Total',labelstyle_gen=None,
                  title=None,versus_wavelength=False,logx='auto',
                  logy='auto',extra=None,mfpunit='cm',softbrackets=False):
    """Plots a given cross-sections (default 'Total') found in given files. Set
    mfp=True to plot mean free path rather than cross-sections and set save_fig
    to save output. The parameter 'show' controls display of interactive window
    (default is to show when no saving a figure). label_gen(ifile,metadata) can
    be used to customise the label generator. Set extra to a function object to
    invoke to plot custom curves."""

    _plot_begin()
    showMFP=mfp

    if not labelstyle_gen:
        labelstyle_gen = lambda ifile,metadata : ('file %i'%ifile,None,None,None)

    colors = ['r','c', 'm', 'y','k','g','b','orange','yellow']
    pp=[XSectParse.ParseXSectFile.parse(filename) for filename in filenames]

    for ifile,p in enumerate(pp):
        npa=numpy.asarray(p['procs'][xsectname])
        if not len(npa):#could be an empty physics list
            continue
        xvals,yvals = _extract_vals(npa,versus_wavelength,showMFP)
        if mfpunit!='cm':
            yvals *= (Units.units.cm/getattr(Units.units,mfpunit))

        label,linestyle,linewidth,col = labelstyle_gen(ifile,p['metadata'])
        if col==None: col=colors[ifile%len(colors)]
        if linestyle==None: linestyle='-'
        plt.plot(xvals,yvals,
                 label=label,color=col,linestyle=linestyle,linewidth=linewidth)

    if title:
        plt.title(title)

    proctxt=' of the process %s'%xsectname if xsectname!='Total' else ''
    if title is not False:
        plt.suptitle(r'Comparison of \texttt{Geant4} %s%s'%('Mean Free Path' if showMFP else 'Cross-sections',proctxt))

    _plot_end(show,save_fig,versus_wavelength,showMFP,logx,logy,extra,mfpunit=mfpunit,softbrackets=softbrackets)

def xsectfile(mat,physlist,particle='neutron',force=False):
    """Get name of xsect file with automatic creation of said file if missing"""
    from Core.System import system
    import os,sys
    fn='xsects_discreteprocs_%s__%s__%s.txt'%(particle,mat,physlist)
    if force or not os.path.exists(fn):
        ec=system('sb_g4xsectdump_query -m"%s" -p%s  -l"%s" -s -f'%(mat,particle,physlist))
        if ec!=0 or not os.path.exists(fn):
            print("ERROR: Could not generate x-sections for %s / %s / %s"%(mat,particle,physlist))
            sys.exit(0)
    return fn
