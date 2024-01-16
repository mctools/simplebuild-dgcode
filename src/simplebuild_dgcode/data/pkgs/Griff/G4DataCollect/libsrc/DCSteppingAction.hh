#ifndef G4DataCollect_DCSteppingAction_hh
#define G4DataCollect_DCSteppingAction_hh

#include "GriffFormat/Format.hh"
#include "G4Interfaces/StepFilterBase.hh"
#include "G4UserSteppingAction.hh"
#include "DCStepData.hh"
#include "Utils/StringSort.hh"
#include <vector>
#include <deque>
class G4Event;
class G4VPhysicalVolume;

namespace G4DataCollectInternals {
  class DCSteppingAction : public G4UserSteppingAction
  {
  public:
    DCSteppingAction(const char* outputFile, GriffFormat::Format::MODE mode, G4UserSteppingAction * otherAction);
    virtual ~DCSteppingAction();
    void UserSteppingAction(const G4Step*);
    void EndOfEventAction(const G4Event*);//This non-standard method will be invoked by our helpful event action.
    G4UserSteppingAction * otherAction() const { return m_otherAction; }
    void setOtherAction(G4UserSteppingAction *ua) { assert(ua&&!m_otherAction); m_otherAction = ua; }
    void setStepFilter(G4Interfaces::StepFilterBase *sf) { assert(sf&&!m_stepFilter); m_stepFilter = sf; m_doFilter=true; }
    void setStepKillFilter(G4Interfaces::StepFilterBase *sf) { assert(sf&&!m_stepKillFilter); m_stepKillFilter = sf; m_doFilter=true; }
    void setMetaData(const std::string& ckey,const std::string& cvalue);
  private:
    GriffFormat::Format::MODE m_mode;
    G4UserSteppingAction * m_otherAction;
    G4Interfaces::StepFilterBase * m_stepFilter;
    G4Interfaces::StepFilterBase * m_stepKillFilter;
    bool m_doFilter;
    int m_prevTrkId;
    int m_prevStepNbr;
    G4VPhysicalVolume* m_prevVol;

    void setBasicMetaData();
    std::vector<std::pair<std::string,std::string>> m_pendingMetaData;
    std::map<std::string,std::string,Utils::fast_str_cmp> m_currentMetaData;
    EvtFile::index_type m_currentMetaDataIdx;

    //File writing managers:
    void initMgr();
    DCMgr * m_mgr;
    std::string m_outputFile;
    struct Track_;

    std::vector<DCStepData*> m_steps;
    std::deque<DCStepData> m_mempool_steps;

    DCStepData * mempoolGetStepObject()
    {
      //"All iterators related to this container are invalidated, but pointers
      //and references remain valid, referring to the same elements they were
      //referring to before the call."
      m_mempool_steps.emplace_back();
#ifdef GRIFF_EXTRA_TESTS
      assert(!m_mempool_steps.back().valid());
#endif
      return &m_mempool_steps.back();
    }
    void clearSteps()
    {
      assert(m_steps.size()==m_mempool_steps.size());
      auto itE=m_steps.end();
      for (auto it=m_steps.begin();it!=itE;++it)
        (*it)->clear();
      m_steps.clear();
      m_mempool_steps.clear();
    }

  };
}

#endif
