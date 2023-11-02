#ifndef DCEventAction_hh
#define DCEventAction_hh

#include "G4UserEventAction.hh"
#include "DCSteppingAction.hh"
#include "G4Utils/Flush.hh"
#include <string>

namespace G4DataCollectInternals {
  class DCEventAction : public G4UserEventAction
  {
  public:

    DCEventAction(DCSteppingAction * sa, G4UserEventAction * otherAct)
      : G4UserEventAction(),
        m_steppingAction(sa),
        m_otherAction(otherAct)
    {
    }

    virtual ~DCEventAction()
    {
    }

    virtual void BeginOfEventAction(const G4Event * evt)
    {
      if (m_otherAction)
        m_otherAction->BeginOfEventAction(evt);
    }

    virtual void EndOfEventAction(const G4Event * evt)
    {
      if (m_otherAction)
        m_otherAction->EndOfEventAction(evt);
      m_steppingAction->EndOfEventAction(evt);
    }

    G4UserEventAction * otherAction() const
    {
      return m_otherAction;
    }

    void setOtherAction(G4UserEventAction *ua)
    {
      assert(!m_otherAction);
      m_otherAction = ua;
    }

  private:
    DCSteppingAction * m_steppingAction;
    G4UserEventAction * m_otherAction;
  };
}

#endif
