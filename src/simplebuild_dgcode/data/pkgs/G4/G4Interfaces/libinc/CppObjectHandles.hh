#ifndef G4Interfaces_CppObjectHandles_hh
#define G4Interfaces_CppObjectHandles_hh

#include <cstdint>
#include <memory>

class G4UserSteppingAction;
class G4UserEventAction;

namespace G4Interfaces {

  class PreGenCallBack;
  class PostGenCallBack;

  //Function object for hooks (for instance those of G4Launcher::Launcher):
  class HookFct {
  public:
    virtual ~HookFct() = default;
    virtual void operator()() = 0;
  };

  //Table of C++ objects (Geant4 user actions and hooks), which are created by
  //one sbldpy based Python module and passed on the Python side to another
  //(typically to G4Launcher). Like components (see ComponentHandles.hh), the
  //objects are referred to by integer handles on the Python side, where they
  //are wrapped in G4Interfaces.ToPy.CppObject objects.
  //
  //The table owns each registered object until it is claimed, which can only
  //happen once and only for the kind of object it was registered as. Objects
  //which are never claimed are deleted when the table itself is destroyed at
  //exit. Example (in a pymod_* module):
  //
  //  m.def( "create_stepact", []() { return G4Interfaces::registerObject( new MySteppingAction ); } );
  //
  //And in the corresponding Python module:
  //
  //  def create_stepact():
  //      return G4Interfaces.ToPy.CppObject( _mymod.create_stepact() )

  std::int64_t registerObject( G4UserSteppingAction* );
  std::int64_t registerObject( G4UserEventAction* );
  std::int64_t registerObject( std::shared_ptr<PreGenCallBack> );
  std::int64_t registerObject( std::shared_ptr<PostGenCallBack> );
  std::int64_t registerObject( std::shared_ptr<HookFct> );

  //Claim objects (the caller takes over the ownership). Throws
  //std::out_of_range for invalid handles, and std::invalid_argument for
  //objects of the wrong kind or objects which were already claimed:
  G4UserSteppingAction* claimUserSteppingAction( std::int64_t handle );
  G4UserEventAction* claimUserEventAction( std::int64_t handle );
  std::shared_ptr<PreGenCallBack> claimPreGenHook( std::int64_t handle );
  std::shared_ptr<PostGenCallBack> claimPostGenHook( std::int64_t handle );
  std::shared_ptr<HookFct> claimHookFct( std::int64_t handle );

  //The kind of object ("G4UserSteppingAction", "G4UserEventAction",
  //"PreGenCallBack", "PostGenCallBack" or "HookFct"), and whether it was
  //already claimed:
  const char * objectKind( std::int64_t handle );
  bool objectIsClaimed( std::int64_t handle );

}

#endif
