#include "MultiProcessingMgr.hh"
#include "G4Interfaces/FrameworkGlobals.hh"
#include "G4Utils/Flush.hh"
#include "Utils/PerfUtils.hh"
#include "Utils/Format.hh"
#include "G4RunManager.hh"
#include "G4Run.hh"
#include <cassert>
#include <sys/wait.h>
#include <signal.h>
#include <stdexcept>
#include <unistd.h>

void G4Launcher::MultiProcessingMgr::scheduleMP(G4Interfaces::ParticleGenBase*gen,unsigned nprocs)
{
  assert(gen);
  //Not using std::make_shared due to private constructor.
  gen->installPreGenCallBack( std::shared_ptr<MultiProcessingMgr>(new MultiProcessingMgr(nprocs)) );
}

G4Launcher::MultiProcessingMgr::MultiProcessingMgr(unsigned nprocs)
  : m_nprocs(nprocs),
    m_nevts_until_abort(0),
    m_first(true),
    m_parentPID(0),
    m_checklasttime(0),
    m_checkinterval(500.0)//milliseconds
{
}

G4Launcher::MultiProcessingMgr::~MultiProcessingMgr()
{
}

std::vector<pid_t> G4Launcher::MultiProcessingMgr::s_childPIDs;

void G4Launcher::MultiProcessingMgr::doFork()
{
  auto rm = G4RunManager::GetRunManager();
  auto nevts = rm->GetCurrentRun()->GetNumberOfEventToBeProcessed();
  if (nevts<1)
    throw std::runtime_error("unexpected nevts < 1");
  if ((long long unsigned)m_nprocs>(long long unsigned)nevts) {
    printf("%sWARNING Limiting number of processes to number of events.\n",FrameworkGlobals::printPrefix());
    m_nprocs=nevts;
  }
  printf("%sForking into %i processes.\n",FrameworkGlobals::printPrefix(),m_nprocs);

  //Must spawn m_nprocs-1 child processes and register the results in
  //G4Interfaces::FrameworkGlobals. The tweaking of per-process stuff like seeds
  //and outputfiles will happen elsewhere (based on the stuff we registered).

  unsigned childs_to_spawn = m_nprocs-1;
  assert(s_childPIDs.empty());
  s_childPIDs.reserve(childs_to_spawn);
  pid_t originalprocpid = getpid();
  unsigned id_next_child = 1;
  bool isparent = true;
  unsigned id_this_process = 0;
  while (childs_to_spawn>=1) {
    //prevent spurious messages by flushing output buffers just before fork:
    G4Utils::flush();
    std::cout.flush();
    //actual fork:
    pid_t childPID = fork();
    if (childPID<0) {
      printf("%sProblems during process fork!\n",FrameworkGlobals::printPrefix());
    }
    if (childPID==0) {
      //This is the child process
      childs_to_spawn=0;
      s_childPIDs.clear();
      isparent=false;
      id_this_process=id_next_child;
    } else {
      //This is the parent process
      s_childPIDs.push_back(childPID);
      --childs_to_spawn;
      ++id_next_child;
    }
  }
  //Done! Register the info:
  FrameworkGlobals::setMpID(id_this_process);
  FrameworkGlobals::setNProcs(m_nprocs);

  std::string tmp;
  std::string tmp2 = FrameworkGlobals::printPrefix();
  if (!tmp2.empty()&&tmp2[tmp2.size()-1]==' ')
    tmp2.resize(tmp2.size()-1);
  Utils::string_format(tmp,"%s[proc%i] ",tmp2.c_str(),id_this_process);
  FrameworkGlobals::setPrintPrefix(tmp.c_str());
  if (!isparent)
    m_parentPID = originalprocpid;
  assert(m_parentPID!=getpid());
  m_checklasttime=PerfUtils::get_cpu_ms();

  //Difficult to change number of events at this point, but we can make sure we
  //trigger soft aborts when it is time:
  if (m_nprocs!=1)
    m_nevts_until_abort = nevts/m_nprocs + (id_this_process<nevts%m_nprocs?1:0);
  else
    m_nevts_until_abort = std::numeric_limits<G4int>::max();
}

void G4Launcher::MultiProcessingMgr::preGen()
{
  if (m_first) {
    m_first=false;
    doFork();
  } else {
    //parent (child) checks occasionally on status of children (parent), to be
    //sure we don't waste time finishing for a long time after a child crashed or
    //was killed.
    double cputime = PerfUtils::get_cpu_ms();
    if (cputime - m_checklasttime >= m_checkinterval) {
      m_checklasttime=cputime;
      if (m_parentPID)
        checkParent();
      else
        checkAnyChildren();
    }
  }
  if ( m_nevts_until_abort-- == 1 )
    G4RunManager::GetRunManager()->AbortRun(true);
}

void G4Launcher::MultiProcessingMgr::killAllChildren()
{
  //... won't somebody think of the CHILDREN???
  auto itE=s_childPIDs.end();
  for (auto it=s_childPIDs.begin();it!=itE;++it)
    kill(*it,SIGKILL);
}

void G4Launcher::MultiProcessingMgr::checkParent()
{
  //Use kill with signal 0 to detect the presence of a given pid:
  //
  //"If sig is 0, then no signal is sent, but error checking is still performed;
  //this can be used to check for the existence of a process ID or process group
  //ID."
  if (kill(m_parentPID, 0)==-1) {
    //Apparently the parent process disappared!
    printf("%sDetected problem in parent process proc0! Exiting.\n",FrameworkGlobals::printPrefix());
    exit(1);
  }
}

void G4Launcher::MultiProcessingMgr::checkAnyChildren(bool wait_finish)
{
  if (!FrameworkGlobals::isForked()||!FrameworkGlobals::isParent())
    return;
  int chld_state;
  for (unsigned ichild = 0; ichild<s_childPIDs.size();++ichild) {
    chld_state = 0;
    pid_t ret = waitpid(s_childPIDs[ichild],&chld_state,wait_finish ? 0 : WNOHANG);
    if (ret==0 && !wait_finish)
      continue;//info not available and we don't want to wait
    if (WIFEXITED(chld_state)) {
      if (WEXITSTATUS(chld_state)==0)
        continue;//exited normally with return code 0
      printf("%sDetected problem in child process proc%i (ended with return code %i)! Exiting.\n",
             FrameworkGlobals::printPrefix(),ichild+1,WEXITSTATUS(chld_state));
      killAllChildren();
      exit(WEXITSTATUS(chld_state));
    } else if (WIFSIGNALED(chld_state)) {
      //child disappeared due to some signal. But we double-check (with
      //kill(..,0))that the PID does not exists anymore:
      if (kill(s_childPIDs[ichild], 0)==-1) {
        printf("%sDetected child process proc%i ended due to signal %i! Exiting.\n",
               FrameworkGlobals::printPrefix(),ichild+1,WTERMSIG(chld_state));
        killAllChildren();
        exit(1);
        //NB: Find signal numbers in: /usr/include/bits/signum.h
      }
    } else {
      killAllChildren();
      printf("%sProblems querying fate of child process proc%i! Exiting.\n",
             FrameworkGlobals::printPrefix(),ichild+1);
      exit(1);
    }
  }
}

