"""Module for creating scan jobs to be launched either at a cluster or locally
"""

import sys
import os

import Core.System
from shlex import quote
import G4Utils.hash2seed

from ScanUtils.ParameterGroup import ParameterGroup

class ScanLauncher:
    def __init__(self,app_name,autoseed=False):
        #autoseed is useful for sim jobs, to prepend --seed=<hashofparams> (taking job multiplicity into account as well)
        self._app_name = app_name
        self._jobpars=[]
        self._global_pars=[]
        self._global_postpars=[]
        self._autoseed=autoseed

    def _create_rundir_and_script(self,rundir,cmd):
        os.mkdir(rundir)
        from Utils.runscript import create_run_script
        create_run_script(os.path.join(rundir,'run.sh'),cmd=cmd,
                          headerlines=['Run script created by ScanUtils.ScanLauncher for local running'])
    def add_global_parameter(self,p,val=None):
        if val is None:
            self._global_pars+=[p]
        else:
            self._global_pars+=['%s=%s'%(p,str(val))]
    def add_global_post_parameter(self,p,val=None):
        if val is None:
            self._global_postpars+=[p]
        else:
            self._global_postpars+=['%s=%s'%(p,str(val))]
    def add_job(self,parameters,label,multiplicity=1):
        mps=list(range(1,multiplicity+1))
        if isinstance(parameters,ParameterGroup):
            for p in parameters.cmd_line_args():
                for mp in mps:
                    self._jobpars+=[(label,p,mp)]
        else:
            for mp in mps:
                self._jobpars+=[(label,parameters,mp)]
    def go(self):
        opt = _parse(self._app_name)
        if opt.show:
            print("Jobs which would be launched by this script:\n")
            counts={}
            for i,(label,cmd) in enumerate(self.job_cmds()):
                counts[label] = 1 + counts.setdefault(label,0)
                print("Job %i (%s): %s"%(i+1,label,_joinargs(cmd)))
            print("Number of jobs by label:")
            for lbl,c in sorted(counts.items()):
                print("  %20s : %i"%(lbl,c))
            print("              => total : %i"%(i+1))
            return
        if opt.launch:
            if not os.path.exists(opt.basedir):
                assert not opt.resubmit
                os.mkdir(opt.basedir)
                print("Created directory for rundirs and output: %s"%opt.basedir)
            else:
                if not opt.resubmit:
                    assert Core.System.isemptydir(opt.basedir)
                    print("Using existing empty directory for rundirs and output: %s"%opt.basedir)
                else:
                    print("Continuing submission in existing directory: %s"%opt.basedir)
            instdir=os.path.join(opt.basedir,'dgcode_install')
            if not opt.resubmit:
                #print("Invoking simplebuild and installing to %s"%instdir)
                #ec=Core.System.system("whatever command here to create a dedicated simplebuild cache-dir in instdir and use it")
                ec=0
                #print('WARNING: The feature preventing future developments from interfering with running jobs is in maintenance,'
                #      '\n         therefore you are not supposed to alter the application while the scan script is running!')
                #print("SKIPPED: Invoking simplebuild and installing to %s"%instdir)
                instdir=os.environ.get('SBLD_INSTALL_PREFIX', None)
                if not ec==0:
                    print("ERROR: Build or installation failed")
                    sys.exit(1)
            if opt.queue=='local':
                print("Preparing job scripts and rundirs")
            njobs=len(self._jobpars)
            jobdirpattern='job%06i'
            for i,(label,cmd) in enumerate(self.job_cmds()):
                rundir=os.path.join(opt.basedir,jobdirpattern%(i+1))
                if opt.queue=='local':
                    self._create_rundir_and_script(rundir,cmd)
                    fh=open(os.path.join(rundir,'comment.txt'),'w')
                    fh.write(label)
                    fh.close()
                elif opt.queue.startswith('dmsc:'):
                    if opt.resubmit and os.path.isdir(rundir):
                        continue
                    print("Submitting job %i/%i (%s) [%g%% done]"%(i+1,njobs,label,(i+1)*100.0/njobs))
                    ec=Core.System.system(_joinargs(['sb_dmscutils_submit',
                                                     opt.queue.split(':')[1],
                                                     '--email=%s'%quote(opt.email),
                                                     '--comment=%s'%quote(label),
                                                     '--name=%s'%os.path.basename(rundir),
                                                     rundir]+cmd))
                    if ec!=0:
                        print("ERROR: Failure during job submission. Aborting!")
                        sys.exit(1)
                else:
                    assert False
            if opt.queue=='local':
                #use make to launch jobs locally and in parallel
                mf=os.path.join(opt.basedir,'Makefile_runjobs')
                fh=open(mf,'w')
                fh.write('BASEDIR:=%s\n\nall:'%quote(opt.basedir))
                for i in range(njobs):
                    fh.write(' job%i'%(i+1))
                fh.write('\n\n')
                for i in range(njobs):
                    fh.write('job%i:\n\t@cd ${BASEDIR}/%s && echo "  Launching job %i / %i (%.1f%%)" && chmod +x run.sh && ./run.sh\n'%(i+1,
                                                                                                                                        jobdirpattern%(i+1),
                                                                                                                                        i+1,
                                                                                                                                        njobs,
                                                                                                                                        (i+1)*100.0/njobs))
                fh.close()
                print("Launching %i jobs using up to %i parallel processes"%(njobs,opt.nprocs))
                kg = '' if opt.haltonerror else ' --keep-going'
                ec=Core.System.system("make -B -j%i%s -f %s"%(opt.nprocs,kg,quote(mf)))
                if ec!=0:
                    print("ERROR: Some jobs failed!")
                else:
                    print("All jobs successful.")
            return

    def job_cmds(self):
        for label,jobpars,multiplicity_index in self._jobpars:
            if self._autoseed:
                seedstr='<;>'.join(jobpars+['__multindex__%i'%multiplicity_index])
                seedpar = [G4Utils.hash2seed.hash2seedpar(seedstr)]
            else:
                seedpar = []
            #put seed first, so any specified seed will take precedence
            yield label,[self._app_name]+self._global_pars+seedpar+jobpars+self._global_postpars


def _joinargs(args):
    return ' '.join(quote(str(a)) for a in args)

def _parse(appname):
    #TODO: The optparse module is deprecated - we should stop using it at some
    #point:
    from optparse import OptionParser,OptionGroup

    parser = OptionParser(usage='%prog [options]',
                          description='Scan script which is used to launch the %s application with various parameters.'%appname
                          +' Note that the feature preventing future developments from interfering with running jobs'
                          +' is in maintenance, therefore you are not supposed to alter the application while the scan'
                          +' script is running, in order to ensure the consistency of the results!')
                          #+' Note that it will invoke \"sb --somethingnotimplemented-install\" before launching jobs, in order to ensure'
                          #+' build consistency and to prevent future developments from interfering with running jobs')

    group_main = OptionGroup(parser, "Main options")
    group_main.add_option("-s", "--show",
                          action="store_true", dest="show", default=False,
                          help="Show which jobs would be launched by --launch option")
    group_main.add_option("--launch",#on purpose no short option, to prevent inadvertent launches
                          action="store_true", dest="launch", default=False,
                          help="Actually launch jobs")
    parser.add_option_group(group_main)
    group_launch = OptionGroup(parser, "Controlling launch aspects")
    queues=['local','dmsc:verylong','dmsc:short','dmsc:newlong','dmsc:quark','dmsc:neutronics']
    group_launch.add_option("-q","--queue",
                            dest="queue", default=queues[0],metavar="QUEUE",
                            help="Queue in which to launch jobs. Valid options are \"%s\""%'", "'.join(queues))
    group_launch.add_option("-d", "--dir",metavar="DIR",
                            dest="basedir", default=None,
                            help="Non-existing or empty directory which will hold run-dirs of scan jobs")
    parser.add_option_group(group_launch)

    group_dmsc = OptionGroup(parser, "Options for DMSC queues only")
    group_dmsc.add_option("-e", "--email",metavar="EMAIL",
                          dest="email", default=None,
                          help="Email to alert upon job failures. Will read attempt to read $EMAIL env var if not supplied.")
    group_dmsc.add_option("-r", "--resubmit",action="store_true",
                          dest="resubmit", default=False,
                          help="Enable to attempt to continue an earlier failed submission attempt. Will not reinstall dgcode.")
    parser.add_option_group(group_dmsc)

    group_local = OptionGroup(parser, "Options for local queue only")
    group_local.add_option("-j",metavar="N",type="int",dest="nprocs", default=None,
                            help="For \"local\" queue only, this sets the number of jobs to run in parallel")
    group_local.add_option("--halt-on-error",
                            action="store_true", dest="haltonerror", default=False,
                            help="For \"local\" queue only, this option prevents further jobs from being launched if any of them halts with an error.")
    parser.add_option_group(group_local)

    (opt, args) = parser.parse_args()
    if args:
        parser.error("Unknown arguments: %s"%' '.join(args))

    if opt.show and opt.launch:
        parser.error('Only one of --show and --launch can be specified at once.')

    if not opt.show and not opt.launch:
        parser.error('Please specificy one of --show and --launch.')

    if opt.launch:
        if not opt.basedir:
            parser.error('--launch requires --dir=DIR argument')

        opt.basedir=os.path.realpath(opt.basedir)
        if not (opt.queue!='local' and opt.resubmit):
            if not Core.System.isemptydir(opt.basedir):
                if os.path.exists(opt.basedir):
                    parser.error("Already exists and not an empty directory: %s"%opt.basedir)
                if not os.path.exists(os.path.dirname(opt.basedir)):
                    parser.error("Not found: %s"%os.path.dirname(opt.basedir))
                if not os.path.isdir(os.path.dirname(opt.basedir)):
                    parser.error("Not a directory: %s"%os.path.dirname(opt.basedir))
        if not opt.queue in queues:
            parser.error('Queue \"%s\" not among valid options: \"%s\"'%(opt.queue,'", "'.join(queues)))
        if opt.queue=='local':
            if opt.nprocs==None:
                parser.error('When using "local" queue, please specify number of parallel jobs with -jN')
            if opt.nprocs<1 or opt.nprocs>1024:
                parser.error('Argument to -j must be integer between 1 and 1024')
        else:
            if opt.email==None:
                opt.email=os.getenv('EMAIL')
            if not '@' in opt.email:
                parser.error('Must specify email address with --email or $EMAIL environment variable when submitting to non-local queue')
    return opt
