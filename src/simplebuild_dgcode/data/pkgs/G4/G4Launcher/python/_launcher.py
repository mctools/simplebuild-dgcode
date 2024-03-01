__doc__="Geant4 launcher"
from . _init import *
def _extrapars(self):
    if not hasattr(self,'_extrapars_obj'):
        self._extrapars_obj = LauncherExtraPars()
    return self._extrapars_obj
Launcher._extrapars = _extrapars
Launcher._keepalive = []

def _create_f(n):
    def f(self,*a,**kw):
        ep=self._extrapars()
        if n.startswith('get'):
            ep.swallowCmdLine()#fixme, locking will give issues!!
        return getattr(ep,n)(*a,**kw)
    f.__name__=n
    return f
for l in ('addParameter%s','getParameter%s'):
    for t in ('String','Boolean','Int','Double'):
        setattr(Launcher,l%t,_create_f(l%t))

#def _clean_expr(x):
#    import re
#    return re.sub('\W|^(?=\d)','_', x)

def _display_mcpl_help():
    print("Capture and write simulated particles to standardised MCPL files")
    print("by supplying one or more flags according to the following syntax")
    print("(note the usage of quotation marks)")
    print()
    print("    --mcpl=\"KEYWORDS [where CONDITION] [to FILENAME] [withflags FLAGS]\"")
    print()
    print("KEYWORDS is a comma separated list containing both names of the")
    print("Geant4 volume in which G4Step's of particles will be considered")
    print("by the MCPLWriter instance being configured, and/or the follow-")
    print("ing list of special keywords:")
    print()
    print("   on_vol_entry  : Only treat G4Step's at volume entry [default]")
    print("   on_vol_exit   : Instead treat G4Step's at volume exit")
    print("   kill_always   : Tracks always killed upon treatment [default]")
    print("   kill_never    : Tracks never killed")
    print("   kill_filtered : Tracks killed only if filter passed")
    print("   all_vols      : Consider all volumes in active geometry")
    print("   opt_dp        : Store double-precision numbers in output.")
    print("   opt_pol       : Store polarisation info in output.")
    print("   opt_userflags : Store special user flags in output.")
    print("   grabsrc       : Short-cut to grab just particles from the particle")
    print('                   generator. Implies kill_never and all_vols plus')
    print('                   prepends "trk.is_primary && step.stepnbr==1" to')
    print('                   CONDITION')
    print()
    print("CONDITION is an optional filter expression based on the G4ExprParser.")#TODO: Wiki link!
    print()
    print("FILENAME is the name of the output file (defaults to \"particles\" if")
    print("not provided).")
    print()
    print("FLAGS is an expression generating a value for the 32bit integer user")
    print("flag field in the output file.")
    print()
    print("Examples:")
    print()
    print('1) Capture (and halt further simulation of) any particle entering')
    print('   volumes named "Target" to a file named "particles.mcpl.gz":')
    print()
    print('    --mcpl=Target')
    print()
    print("2) Capture generated neutrons to srcneutrons.mcpl.gz:")
    print()
    print('    --mcpl="grabsrc where trk.is_neutron to srcneutrons.mcpl.gz"')
    print()
    print('3) Capture high energy gammas entering volumes named "Detector"')
    print('   or "Sample":')
    print()
    print('    --mcpl="Detector,Sample where trk.pdgcode==22 and step.pre.ekin > 1MeV"')
    print()
    print('4) Capture any particle leaving the "World" volume with double-')
    print('   precision storage:')
    print()
    print('    --mcpl="World,opt_dp,on_vol_exit"')
    print()
    print('5) Capture any particle entering volumes named "Detector" and embed copy')
    print('   number information about the Detector volume and it\'s mother volume')
    print('   into the user flag field of the mcpl file:')
    print()
    print('    --mcpl="World withflags step.volcopyno+1000*step.volcopyno_1"')
    print()
    import sys
    sys.exit(0)

def _display_heatmap_help():
    #NB: We don't mention the --heatmap=density option at all for now.
    print("Produce mesh3d files with extracted quantities from simulated particle")
    print("steps, by supplying an argument with the following syntax:")
    print()
    print("    --heatmap=\"QUANTITY [where CONDITION] [to FILENAME]\"")
    print()
    print("QUANTITY is an expression based on the G4ExprParser.")#todo: wiki link
    print()
    print("CONDITION is an optional filter expression based on the G4ExprParser.")
    print()
    print("FILENAME is the name of the output file (defaults to \"heatmap\" if")
    print("not provided). Append :(nx,ny,nz) to the FILENAME to modify binning.")
    print()
    print("Supplying --heatmap with no arguments implies --heatmap=step.edep")
    print()
    print("Examples:")
    print()
    print('1) Simply get a map of energy depositions:')
    print()
    print('    --heatmap=step.edep')
    print()
    print("2) Same, but only for electrons and gamma and stored in edep_em.mesh3d:")
    print()
    print('    --heatmap="step.edep where trk.pdgcode==11||trk.is_photon to edep_em"')
    print()
    print('3) Get approximate energy-flux map by calculating the average')
    print('   kinetic energy across the step and scaling with its length:')
    print()
    print('    --heatmap="0.5*(step.pre.ekin+step.post.ekin)*step.steplength"')
    print()
    import sys
    sys.exit(0)

def _configure_mcplwriter(launcher,cfgstr,parseerror):
    if not cfgstr or cfgstr.strip()=='help':
        _display_mcpl_help()
    from G4MCPLPlugins.MCPLWriter import MCPLWriter
    if not hasattr(launcher,'_mcpl_outputs'):
        setattr(launcher,'_mcpl_outputs',set())
    mcpl_fltr,mcpl_fn,flagexpr = None,None,None
    cfgstr=' %s '%cfgstr#catch ' where ' and ' to ' also in malformed strings starting with 'where ...'
    if ' to ' in cfgstr:
        cfgstr, mcpl_fn = cfgstr.split(' to ',1)
        mcpl_fn=mcpl_fn.strip()
        if not mcpl_fn:
            parseerror('Malformed option for --mcpl: " to " must be followed by a filename')
    if not mcpl_fn:
        mcpl_fn = 'particles'
    mcpl_fn = mcpl_fn.strip()
    if ' ' in mcpl_fn:
        parseerror('Malformed option for --mcpl: spaces in filename.')
    if ' where ' in cfgstr:
        cfgstr, mcpl_fltr = cfgstr.split(' where ',1)
        mcpl_fltr=mcpl_fltr.strip()
        if not mcpl_fltr:
            parseerror('Malformed option for --mcpl: " where " must be followed by a filter expression')
    if ' withflags ' in cfgstr:
        cfgstr, flagexpr = cfgstr.split(' withflags ',1)
        flagexpr=flagexpr.strip()
        if not flagexpr:
            parseerror('Malformed option for --mcpl: " withflags " must be followed by a filter expression')
    cfgs=set(cfgstr.strip().split(','))
    if 'help' in cfgs:
        _display_mcpl_help()
    while mcpl_fn.endswith('.gz'): mcpl_fn = mcpl_fn[0:-3]
    while mcpl_fn.endswith('.mcpl'): mcpl_fn = mcpl_fn[0:-5]
    tmp,i=mcpl_fn,2
    while tmp in launcher._mcpl_outputs:
        tmp = '%s_%i'%(mcpl_fn,i)
        i+=1
    mcpl_fn = tmp
    launcher._mcpl_outputs.add(mcpl_fn)
    mw = MCPLWriter(mcpl_fn+'.mcpl')
    ks_default='NEVER' if 'grabsrc' in cfgs else 'ALWAYS'#default to avoid unintentional double-counting
    on_exit_default = False
    ks, on_exit = [], []
    opt_dp,opt_pol,opt_userflags = False,False,False
    for cfg in cfgs:
        cfg = cfg.strip()
        if not cfg:
            continue
        if cfg=='help': _display_mcpl_help()
        elif cfg=='grabsrc':
            mw.addVolume('*')#all volumes
            if mcpl_fltr: mcpl_fltr = 'trk.is_primary && ( step.stepnbr==1 && (%s) )'%mcpl_fltr
            else: mcpl_fltr = 'trk.is_primary && step.stepnbr==1'
        elif cfg=='kill_always': ks += ['ALWAYS']
        elif cfg=='kill_never': ks+=['NEVER']
        elif cfg=='kill_filtered': ks+=['FILTERED']
        elif cfg=='on_vol_exit': on_exit += [True]
        elif cfg=='on_vol_entry': on_exit += [False]
        elif cfg=='opt_dp': opt_dp = True
        elif cfg=='opt_pol': opt_pol = True
        elif cfg=='opt_userflags': opt_userflags = True
        elif cfg=='all_vols': mw.addVolume('*')
        elif cfg=='where' or cfg=='to' or cfg.startswith('kill_') or cfg.startswith('on_vol_') or cfg.startswith('opt_') or cfg=='withflags':
            parseerror('Suspicious volume name "%s" indicates malformed option for --mcpl'%cfg)
        else:
            mw.addVolume(cfg)
    if len(set(ks))>1: parseerror('mutually exclusive kill_xxx flags specified')
    if len(set(on_exit))>1: parseerror('mutually exclusive on_vol_exit/on_vol_entry flags specified')
    mw.setWriteDoublePrecision(opt_dp)
    mw.setWritePolarisation(opt_pol)
    mw.setWriteUserFlags(opt_userflags)
    mw.setWriteOnVolExit(on_exit[0] if on_exit else on_exit_default)
    mw.setKillStrategy(ks[0] if ks else ks_default)
    if mcpl_fltr:
        mw.setFilter(mcpl_fltr)
    if flagexpr:
        mw.setUserFlags(flagexpr)

    launcher.addPostInitHook(mw.inithook)

def _configure_heatmap(launcher,cfgstr,parseerror):
    if not cfgstr or cfgstr.strip()=='help':
        _display_heatmap_help()
    from G4HeatMap import HeatMapWriter
    if not hasattr(launcher,'_heatmap_outputs'):
        setattr(launcher,'_heatmap_outputs',set())
    quantity,condition,filename = None,None,None
    cfgstr=' %s '%cfgstr#catch ' where ' and ' to ' also in malformed strings starting with 'where ...'
    binning = [200,200,200]
    if ' to ' in cfgstr:
        cfgstr, filename = cfgstr.split(' to ',1)
        filename=filename.strip()
        if not filename:
            parseerror('Malformed option for --heatmap: " to " must be followed by a filename')
        if ':' in filename:
            import re
            filename,binning = filename.split(':',1)
            m = re.match(r'\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)\s*', binning)
            if not m or not len(m.groups(1))==3:
                parseerror('Malformed option for --heatmap: invalid binning')
            binning = [min(1000000000,max(1,int(e))) for e in m.groups(1)]

    if not filename:
        filename = 'heatmap'
    filename = filename.strip()
    if ' ' in filename:
        parseerror('Malformed option for --heatmap: spaces in filename.')
    if ' where ' in cfgstr:
        quantity, condition = cfgstr.split(' where ',1)
        condition=condition.strip()
        if not condition:
            parseerror('Malformed option for --heatmap: " where " must be followed by a filter expression')
    else:
        quantity = cfgstr
    quantity=quantity.strip()
    if not quantity:
        parseerror('Malformed option for --heatmap: Missing quantity')
    if quantity=='help':
        _display_heatmap_help()
    while filename.endswith('.mesh3d'):
        filename = filename[0:-7]
    tmp,i=filename,2
    while tmp in launcher._heatmap_outputs:
        tmp = '%s_%i'%(filename,i)
        i+=1
    filename = tmp
    launcher._heatmap_outputs.add(filename)
    hm = HeatMapWriter(filename+'.mesh3d',*binning)
    hm.setQuantity(quantity)
    if condition:
        hm.setFilter(condition)
    launcher.postinit_hook(hm.inithook)

def _swallowCmdLineAndLaunch(self):
    extraparlist = self._extrapars().getParameterList()
    if extraparlist:
        self._extrapars().swallowCmdLine()#just to get rid of them
    import sys,os
    import argparse
    self.allowMultipleSettings()
    #default values:
    default_mp = self.getMultiProcessing()
    if default_mp<2: default_mp=1
    default_visengine = self.getVis()
    if default_visengine:
        default_dovis=True
    else:
        default_dovis=False
        default_visengine='OGL'
    default_mode=self.getOutputMode()
    default_outfile=self.getOutputFile()
    if not default_mode: default_outfile='FULL'
    if not default_outfile: default_outfile='simresults'

    default_physicsList=self.getPhysicsList()

    norandom = self.GetNoRandomSetup();
    default_seed = self.getSeed();
    if not default_seed:
        default_seed = 123456789

    geo = self.getGeo()
    gen = self.getGen()
    unlimited_src = gen.unlimited() if gen else True
    flt = self.getFilter()
    assert geo or gen,"neither geometry nor generator has been set"
    gendescr= 'particles from the %s generator hitting '%gen.getName() if gen else ''
    geodescr= 'the %s geometry'%geo.getName() if geo else ''

    pn=os.path.basename(sys.argv[0])

    class HF(argparse.HelpFormatter):
        #Trick to get '--longopt=arg' shown in help rather than '--longopt arg'
        #(in principle _format_action_invocation is unstable API, but we ship a
        #fixed argparse.py so it won't change all of a sudden):
        def _format_action_invocation(self,action):
            res=argparse.HelpFormatter._format_action_invocation(self,action)
            if action.metavar and any(a.startswith('--') for a in action.option_strings):
                for a in action.option_strings:
                    if a.startswith('--'):
                        if action.nargs=='?':
                            res=res.replace('%s ['%a,'%s[='%a)
                        else:
                            res=res.replace('%s '%a,'%s='%a)
            return res

    parser = argparse.ArgumentParser(prog=pn,usage='%s [options] [par1=val1] [par2=val2] [...]'%pn,
                                     formatter_class=HF,
                                     description=('This script allows you to simulate or visualise %s%s.'+
                                                  ' Note that in addition to the options below, you can override'+
                                                  ' parameters of the generator and geometry by supplying them on the commandline like par=val.'+
                                                  ' Furthermore note that as a special case, you can disable the parameter validation by setting'+
                                                  ' forcepars=yes.')%(gendescr,geodescr))
    parser.add_argument("-d", "--dump", action='store_true',default=False,dest="dump",
                        help='Dump parameters of both geometry%s'%(', generator and filter' if flt else ' and generator'))
    parser.add_argument("-g", action='store_true',default=False,dest="dumpgeo",
                        help='Dump parameters of just geometry')
    parser.add_argument("-p", action='store_true',default=False,dest="dumpgen",
                        help='Dump parameters of just generator')
    if extraparlist:
        parser.add_argument("--extrapars", action='store_true',default=False,dest="dumpep",
                            help='Dump extra parameters from Launcher script')
    if flt:
        parser.add_argument("-f", action='store_true',default=False,dest="dumpflt",
                            help='Dump parameters of just filter')
    parser.add_argument("-x", action='store_true',default=False,dest="dumpxsects",
                        help='Dump used cross-sections in text files')

    parser.add_argument("--viewer", action='store_true',default=False,dest="osgviewer",
                        help='Experimental custom geometry visualisation')

    parser.add_argument("--dataviewer", action='store_true',default=False,dest="osgviewer_data",
                        help='Experimental visualisation of both geometry and data')
    parser.add_argument("--aimdataviewer", action='store_true',default=False,dest="osgviewer_data_aim",
                        help='Like --dataviewer, but showing just first segment of primary '
                        +'tracks')
    parser.add_argument('--heatmap', type=str, dest="heatmap", const='step.edep',nargs='?',
                        metavar='CFG',action='append',
                        help='Collect quantities from simulation steps into mesh3d file. Use --heatmap=help for detailed instructions.')
    parser.add_argument('--mcpl', type=str, dest="mcpl",
                        metavar='CFG',action='append',
                        help='Capture and write simulated particles to standardised MCPL files. Use --mcpl=help for detailed instructions.')
    parser.add_argument("-n", "--nevts",type=int, dest="nevts", default=(10 if unlimited_src else 0),
                        help="Simulate N events",metavar="N")
    parser.add_argument("-j", "--jobs",type=int, dest="njobs", default=default_mp,
                        help="Launch N processes [default %i]"%default_mp,metavar="N")

    parser.add_argument("-t", "--test", action='store_true',default=False,dest="test",
                        help='Test geometry consistency and exit')
    parser.add_argument("-l", "--physlist",type=str, dest="physicslist", default=default_physicsList,
                        help="Physics List [default %s]"%default_physicsList,metavar='PL')
    parser.add_argument("--showphysicslists",action='store_true',default=False,dest="showphyslist",
                        help="Show available physics lists")
    parser.add_argument("--allowfpe",action='store_true',default=False,dest="allowfpe",
                        help="Do not trap floating point errors")
    if not norandom:
        parser.add_argument("-s", "--seed",type=int, dest="seed", default=default_seed,
                            help="Use S as seed for generation of random numbers [default %i]"%default_seed,metavar='S')
    if default_dovis:
        parser.add_argument("-n", "--novisualise",action='store_false',default=True,dest="vis",
                            help='Do *not* drop to G4 interactive prompt and launch viewer')
    else:
        parser.add_argument("-v", "--visualise",action='store_true',default=False,dest="vis",
                            help='Drop to G4 interactive prompt and launch viewer')
    parser.add_argument("-e", "--engine",type=str,default=default_visengine,dest="visengine",metavar='ENG',
                        help='Use visualisation engine ENG [default %s]'%default_visengine)
    parser.add_argument("-i", "--interactive",action='store_true',default=False,dest="interactive",
                        help='Drop to G4 interactive prompt')
    parser.add_argument("-r","--verbose",action='count',dest="trackingverbose",
                        help='Enables tracking printouts (specify multiple times for ever increasing levels of details, up to -rrrrrrr))')
    parser.add_argument("-o", "--output",type=str, dest="outfile", default=default_outfile,
                        help="Filename for GRIFF output [default %s]"%default_outfile,metavar='FN')
    parser.add_argument("-m", "--mode",type=str,choices=['FULL','REDUCED','MINIMAL'], dest="mode",default=default_mode,metavar='MODE',
                        help="GRIFF storage mode [default %s]"%default_mode)
    #Don't feed custom args of the form name=val to the parser:
    args_custom=set([a for a in sys.argv[1:] if (not a.startswith('-') and '=' in a)])
    (opt, args) = parser.parse_known_args([a for a in sys.argv[1:] if not a in args_custom])
    args += list(args_custom)

    #Handle aliases:
    if opt.physicslist == 'empty':
        opt.physicslist = 'PL_Empty'
    if opt.physicslist.startswith('ESS_'):
        plnametry = 'PL_' + opt.physicslist[len('ESS_'):]
        print('G4Launcher:: WARNING Trying to map deprecated physics list'
              f' name {opt.physicslist} to {plnametry}')
        opt.physicslist = plnametry

    if opt.osgviewer_data_aim:
        opt.osgviewer_data=True
    if opt.osgviewer_data:
        opt.osgviewer = False

    if opt.showphyslist:
        import G4PhysicsLists
        G4PhysicsLists.printAvailableLists()
    if geo:
        geo.swallowCmdLine(args)
    if gen:
        gen.swallowCmdLine(args)
    if flt:
        flt.swallowCmdLine(args)

    #Now we can remove any lingering 'forcepars=...' arguments:
    for a in [a for a in args if a.startswith('forcepars=')]:
        args.remove(a)

    if args: parser.error('Unknown arguments: %s'%' '.join(args))

    if opt.allowfpe:
        self.allowFPE()

    if not norandom:
        if opt.seed<0: parser.error('Seed must be a positive number')
        if opt.seed!=default_seed:
            self.setSeed(opt.seed)
    if opt.njobs<1: parser.error('Number of parallel processes must be at least 1')

    if unlimited_src:
        if opt.nevts<1:
            parser.error('Number of events should be at least 1'+
                         (' (--nevts=0 with this generator would result in a job which would never finish)' if opt.nevts==0 else ''))
    else:
        if opt.nevts<0:
            parser.error('Number of events should not be negative (use --nevts=0 to run until generator runs out of events)')

    if opt.dump or opt.dumpgeo:
        if geo: geo.dump()
        else: print("%sWARNING: No geometry set. Dump aborted."%self.getPrintPrefix())
    if opt.dump or opt.dumpgen:
        if gen: gen.dump()
        else: print("%sWARNING: No generator set. Dump aborted."%self.getPrintPrefix())
    if flt and (opt.dump or opt.dumpflt):
        if flt: flt.dump()
        else: print("%sWARNING: No filter set. Dump aborted."%self.getPrintPrefix())
    if extraparlist and (opt.dump or opt.dumpep):
        print("LauncherPars:")
        self._extrapars().dump("  ")

    if opt.dump or opt.dumpgen or opt.dumpgeo or (flt and opt.dumpflt) or (extraparlist and opt.dumpep) or opt.showphyslist:
        return self._shutdown()#shutdown: Make sure run-manager deletion happens now and not when garbage collection runs

    if opt.osgviewer and opt.physicslist!='PL_Empty':
        print('%s Geometry visualisation requested => Will use PL_Empty physics list'%self.getPrintPrefix())
        opt.physicslist='PL_Empty'

    self.setPhysicsList(opt.physicslist)

    if opt.test:
        if not geo:
            print("%sWARNING: No geometry set. Test aborted."%self.getPrintPrefix())
            sys.exit(1)
        self.cmd_postinit("/geometry/test/tolerance 0.000001 mm")
        self.cmd_postinit("/geometry/test/run")
        self.init()
        return self._shutdown()#shutdown: Make sure run-manager deletion happens now and not when garbage collection runs

    if opt.vis:
        self.setVis(opt.visengine)

    if opt.dumpxsects:
        try:
            import G4XSectDump.XSectSpy as spy
        except:
            print("\n\n\nERROR: Attempting to dump with x-sections requires the package G4XSectDump to be enabled!!!\n\n\n")
            raise
        self.postinit_hook(spy.install)

    from G4Utils import flush as G4Utils_flush
    if opt.physicslist!='PL_Empty':
        #install G4NCrystal on demand (devel version from Projects/NCrystal - framework version is in Launcher.cc)
        try:
            import NCrystalPreview
            self.postinit_hook(NCrystalPreview.installOnDemand)
        except ImportError:
            #NCrystalPreview was not built, ok to fail silently since no NCG4 materials can
            #have been added to the geometry in this case. However, to not break
            #tests when the NCrystalPreview package is enabled/disabled, we fake messages
            #similar to those emitted by G4NCInstall.
            def fake_G4NCInstall():
                G4Utils_flush()
                sys.stdout.flush()
                print('G4NCInstall :: No materials with "NCrystal" property found in active geometry.')
                print('G4NCInstall :: Will not touch existing processes for neutrons')
                sys.stdout.flush()
            self.postinit_hook(fake_G4NCInstall)


    call_post_sim = []
    if opt.osgviewer or opt.osgviewer_data:
        try:
            import G4OSG.Viewer
        except ImportError:
            print("\n\n%sERROR: Failed to import G4OSG.Viewer module (perhaps you do not have OpenSceneGraph installed!)\n\n"%self.getPrintPrefix())
            raise
        if not opt.osgviewer_data:
            #just the geometry:
            def launch_viewer_then_exit():
                print('='*80)
                print("== Launching viewer!")
                print('='*80)
                ec=G4OSG.Viewer.run()
                print('='*80)
                print("==  Viewer done: Exiting.")
                print('='*80)
                import sys
                sys.exit(ec if ec<128 else 127)
            self.postinit_hook(launch_viewer_then_exit)
        else:
            #run nevts and visualise the resulting griff file event by event
            if opt.njobs > 1:
                print("%sWARNING: Disabling requested multiprocessing since --dataviewer/--aimdataviewer was specified!"%self.getPrintPrefix())
                opt.nevts *= opt.njobs
                opt.njobs = 1
            print("%sWARNING: Placing output in temporary griff file in FULL mode (ignoring user settings) since --dataviewer/--aimdataviewer was specified!"%self.getPrintPrefix())
            import tempfile
            fn=os.path.abspath(os.path.realpath(tempfile.mktemp(suffix='_vis.griff')))
            from Core.System import remove_atexit
            remove_atexit(fn)
            import GriffDataRead
            GriffDataRead.GriffDataReader.setOpenMsg(False)
            opt.outfile = fn
            opt.mode = 'FULL'
            #geo and event data::
            if opt.osgviewer_data_aim:
                import os
                os.environ['G4OSG_DATAVIEW_AIMING_MODE'] = '1'
            def launch_viewer_with_data():
                self.closeOutput()#ensure Griff file is closed properly!
                if hasattr(self,'_previewer_callback'):
                    self._previewer_callback(self)
                print('='*80)
                print("==  Launching viewer!")
                print('='*80)
                ec=G4OSG.Viewer.run(opt.outfile)
                print('='*80)
                print("==  Viewer done! Hope you enjoyed the experimental event data visualisation :-)")
                print('='*80)
            call_post_sim += [launch_viewer_with_data]

    if opt.vis or opt.interactive:
        self.addHist("/run/beamOn %i"%(opt.nevts if opt.nevts else 10))
        self.startSession()
    else:
        #special secret heatmap option, "density"
        if opt.heatmap and 'density' in opt.heatmap:
            opt.heatmap.remove('density')
            def create_density_map():
                import G4HeatMap.DensityMap as DM
                fn="heatmap_world_density.mesh3d"
                print("HeatMapWriter: Extracting density map to %s"%fn)
                DM.create_density_map(fn,'',100,100,100,100)
            self.postinit_hook(create_density_map)

        if opt.heatmap:
            for cfgstr in opt.heatmap:
                if not cfgstr:
                    parser.error('Missing argument for option --heatmap')
                _configure_heatmap(self,cfgstr,parser.error)

        if opt.mcpl:
            for cfgstr in opt.mcpl:
                if not cfgstr:
                    parser.error('Missing argument for option --mcpl')
                _configure_mcplwriter(self,cfgstr,parser.error)

        if opt.trackingverbose:
            self.cmd_postinit("/tracking/verbose %i"%opt.trackingverbose)
            #ALWAYS display evt msg in this case unless user specifically
            #requested otherwise
            if not norandom and not self.rndEvtMsgMode():
                self.setRndEvtMsgMode('ALWAYS')
        self.setOutput(opt.outfile,opt.mode)
        if opt.njobs!=self.getMultiProcessing():
            self.setMultiProcessing(opt.njobs)
        self.startSimulation(opt.nevts)
        for c in call_post_sim:
            c()
    #todo: directory for outfile must exist (or create?)

    self._shutdown()#shutdown: Make sure run-manager deletion happens now and not when garbage collection runs

##_setPhysicsList_orig = Launcher.setPhysicsList
##def _setPhysicsList(self,physlistname):
##    #Make sure that custom physics lists will be translated into a provider
##    #rather than just the string (this assumes that the default physics list
##    #will not be custom, which it can't be if we want to avoid a direct
##    #dependency on the custom physics list code in question).
##    import G4PhysicsLists
##    if G4PhysicsLists.listIsCustom(physlistname):
##        #extract physics list provider and apply that:
##        self.setPhysicsListProvider(G4PhysicsLists.extractProvider(physlistname))
##    else:
##        _setPhysicsList_orig(self,physlistname)
##
#Launcher.setPhysicsList = _setPhysicsList

Launcher.swallowCmdLineAndLaunch = _swallowCmdLineAndLaunch
Launcher.go = _swallowCmdLineAndLaunch#alias
