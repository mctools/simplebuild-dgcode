#include "sbldpy/sbldpy.hh"
#include "G4Interfaces/CppObjectHandles.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4UserSteppingAction.hh"
#include "G4UserEventAction.hh"
#include <tuple>

//Module providing one C++ object of each kind supported by
//G4Interfaces/CppObjectHandles.hh, counting how often they are invoked (see
//the testcppobjects script).

namespace {
  struct Counts {
    std::int64_t steps = 0, events = 0, pregen = 0, postgen = 0, hooks = 0;
  };
  Counts& counts()
  {
    static Counts c;
    return c;
  }
  class StepAct : public G4UserSteppingAction {
  public:
    void UserSteppingAction( const G4Step* ) override { ++counts().steps; }
  };
  class EvtAct : public G4UserEventAction {
  public:
    void BeginOfEventAction( const G4Event* ) override { ++counts().events; }
  };
  class PreGen : public G4Interfaces::PreGenCallBack {
  public:
    void preGen() override { ++counts().pregen; }
  };
  class PostGen : public G4Interfaces::PostGenCallBack {
  public:
    void postGen( G4Event* ) override { ++counts().postgen; }
  };
  class Hook : public G4Interfaces::HookFct {
  public:
    void operator()() override { ++counts().hooks; }
  };
}

SBLDPY_MODULE( _CppObjTest )
{
  m.def( "create_stepact", []() { return G4Interfaces::registerObject( new StepAct ); } );
  m.def( "create_evtact", []() { return G4Interfaces::registerObject( new EvtAct ); } );
  m.def( "create_pregen", []() { return G4Interfaces::registerObject( std::make_shared<PreGen>() ); } );
  m.def( "create_postgen", []() { return G4Interfaces::registerObject( std::make_shared<PostGen>() ); } );
  m.def( "create_hook", []() { return G4Interfaces::registerObject( std::make_shared<Hook>() ); } );
  m.def( "counts", []()
  {
    auto& c = counts();
    return std::make_tuple( c.steps, c.events, c.pregen, c.postgen, c.hooks );
  }, "Number of steps, events, pregen calls, postgen calls and hook calls" );
}
