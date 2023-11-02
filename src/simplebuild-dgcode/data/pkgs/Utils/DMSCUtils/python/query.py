import Core.System as Sys
import os

def jobid(jobdir):
    """Get jobid of jobdir for a job submitted with sb_dmscutils_submit. Returns None if unable to determine."""
    if not os.path.isdir(jobdir):
        return None
    if not os.path.exists(os.path.join(jobdir,'state.submit')):
        return None
    so=os.path.join(jobdir,'slurm_submitstdout.txt')
    if not os.path.exists(so):
        return None
    so=' '.join(open(so).read().strip().split())
    if not so.startswith('Submitted batch job'):
        return None
    so=so.split()
    if not len(so)==4 or not so[3].isdigit():
        return None
    return int(so[3])

def iscurrent(jobdir_or_jobid):
    """determine whether job is still in the queue in any state. Returns None if unable to determine."""
    if not isinstance(jobdir_or_jobid,int):
        jobdir_or_jobid = jobid(jobdir_or_jobid)
        if jobdir_or_jobid==None:
            return None
    assert isinstance(jobdir_or_jobid,int)
    #Slurm states are: PENDING (PD), RUNNING (R), SUSPENDED (S), COMPLETING
    #                  (CG), COMPLETED (CD), CONFIGURING (CF), CANCELLED (CA),
    #                  FAILED (F), TIMEOUT (TO), PREEMPTED (PR) and NODE_FAIL (NF).
    #
    #We ask squeue if the job is in PD,R,S,CG or CF, which hopefully are exactly
    #the states connected to jobs that are still "current" in the sense that
    #they didn't end (successfully or otherwise):
    ec=Sys.system('squeue  --job=%i --noheader -t PD,R,S,CG,CF > /dev/null'%jobdir_or_jobid)
    if ec==0:
        #job still in queue
        return True
    else:
        return False

