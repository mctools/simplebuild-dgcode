#ifndef G4Launcher_MultiProcessingMgr_hh
#define G4Launcher_MultiProcessingMgr_hh

#include "G4Interfaces/ParticleGenBase.hh"
#include <sys/types.h>//pid_t
#include "G4Types.hh"
#include <memory>

//If your generator is mygen, then you schedule a fork into N processes by
//performing a call like the following during initialisation:
//
//  G4Launcher::MultiProcessingMgr::scheduleMP(mygen,N);
//
//The actual fork() will happen after initialisation, thus ensuring a very
//efficient memory sharing.

namespace G4Launcher {

  class MultiProcessingMgr : public G4Interfaces::PreGenCallBack {
  public:
    static void scheduleMP(G4Interfaces::ParticleGenBase*,unsigned nprocs);

    //Wait for child procs to finish (if any). Exits the process in case of
    //trouble. If nowait==true it will simply query children rather than wait
    //for them to finish:
    static void checkAnyChildren(bool wait_finish=false);

    virtual ~MultiProcessingMgr();

  private:
    virtual void preGen();
    MultiProcessingMgr(unsigned nprocs);
    static void killAllChildren();
    void checkParent();
    void doFork();
    unsigned m_nprocs;
    G4int m_nevts_until_abort;
    bool m_first;
    pid_t m_parentPID;
    static std::vector<pid_t> s_childPIDs;
    double m_checklasttime;
    double m_checkinterval;
  };

}

#endif
