
__doc__='module for chaining g4 simulation and griff analysis steps'
import os
import sys
import shlex
from Core.System import mkdir_p,system
import G4Utils.hash2seed


def chain(g4sim_exec_name,griffana_exec_name,griff_mode=None):
    def usage(ec):
        pn=os.path.basename(sys.argv[0])
        print("Usage:")
        print("%s <dir> [--chainmult=N] [--cleanup] [-h|--help|--helpsim] <options for %s>"%(pn,g4sim_exec_name))
        print()
        print("Runs first %s in <dir>/runsim and then %s in <dir>/runana"%(g4sim_exec_name,griffana_exec_name))
        print()
        print("Additionally runs a 1 event simulation job in <dir>/runsample (useful when using --cleanup)")
        print()
        print("Griff output will be enabled for %s, and the resulting files will be fed"%g4sim_exec_name)
        print("to %s if the former runs successfully."%griffana_exec_name)
        print()
        print("Options:")
        print("  -h, --help            show this help message and exit")
        print("  --helpsim             show help message of simulation script")
        print("  --cleanup             remove griff files after analysis")
        print("  --chainmult=M         run sim+ana M times with different seeds and merge output .shist files")
        sys.exit(ec)

    args=sys.argv[1:]
    if '-h' in args or '--help' in args:
        usage(0)

    if '--helpsim' in args:
        ec=system("%s --help"%g4sim_exec_name)
        sys.exit(127 if ec > 127 else ec)

    chainmult=1
    for a in args:
        if a.startswith('--chainmult='):
            chainmult=a[12:]
            if not chainmult.isdigit() or int(chainmult)<1:
                usage(1)
            chainmult=int(chainmult)
            args.remove(a)

    #convenience:
    for a in ['-d','--dump','-p','-g','--viewer','--showphysicslists','--extrapars','--extra','-f']:
        if a in args:
            ec=system("%s %s --output=none"%(g4sim_exec_name,a))
            sys.exit(127 if ec > 127 else ec)

    do_cleanup=False
    if '--cleanup' in args:
        do_cleanup=True
        args.remove('--cleanup')
    if not args: usage(1)
    rundir=args[0]
    if '=' in rundir or rundir.startswith('-'):
        usage(1)

    args=args[1:]
    if os.path.exists(rundir):
        print("ERROR: Rundir %s exists"%rundir)
        sys.exit(1)
    if not os.path.isdir(os.path.dirname(os.path.realpath(rundir))):
        print("ERROR: Parent directory of requested rundir does not exist")
        sys.exit(1)
    #if not args:
    #    usage(1)

    #ok, let us get to it!!
    def setup_runscript(rundir,cmd):
        mkdir_p(rundir)
        #os.chdir(rundir)
        fh=open(os.path.join(rundir,'run.sh'),'w')
        fh.write('cd %s && \\\n'%shlex.quote(rundir))
        fh.write(' '.join(shlex.quote(c) for c in cmd)+' >& output.log\n')
        fh.write('ec=$?\n')
        fh.write('if [ -f output.log ]; then gzip output.log; fi\n')
        fh.write('exit $ec\n')
        fh.close()
    preargs=[]
    if griff_mode:
        preargs=['--mode=%s'%griff_mode]
    setup_runscript(os.path.join(rundir,'runsample'),[g4sim_exec_name]+preargs+args+['--nevts=1','--jobs=1','--output=sim.griff'])
    simargs=[g4sim_exec_name]+preargs+args+['--output=sim.griff']
    if chainmult==1:
        setup_runscript(os.path.join(rundir,'runsim'),simargs)
        setup_runscript(os.path.join(rundir,'runana'),[griffana_exec_name,'../runsim/sim*.griff'])
    else:
        for i in range(1,1+chainmult):
            seedpar=G4Utils.hash2seed.hash2seedpar('##'.join(simargs+['chainmult=%i'%i]))
            setup_runscript(os.path.join(rundir,'runsim_part%04i'%i),simargs+[seedpar])
            setup_runscript(os.path.join(rundir,'runana_part%04i'%i),[griffana_exec_name,'../runsim_part%04i/sim*.griff'%i])
        setup_runscript(os.path.join(rundir,'runana'),['sb_simplehists_merge','-o','merged.shist','../runana_part*/*.shist'])
    def run(step):
        ec=system(". %s/run.sh"%shlex.quote(os.path.join(rundir,step)))
        if ec!=0:
            print("ERROR: Failure at %s step"%step)
            sys.exit(ec if ec<127 else 127)
    def cleanup_sim(dirname):
        if do_cleanup:
            ec=system("rm -f %s/sim*.griff"%shlex.quote(os.path.join(rundir,dirname)))
            if ec!=0:
                print("ERROR: Failure during cleanup")
                sys.exit(ec if ec<127 else 127)
    run('runsample')
    if chainmult==1:
        run('runsim')
        run('runana')
        cleanup_sim('runsim')
    else:
        for i in range(1,1+chainmult):
            run('runsim_part%04i'%i)
            run('runana_part%04i'%i)
            cleanup_sim('runsim_part%04i'%i)
        run('runana')
