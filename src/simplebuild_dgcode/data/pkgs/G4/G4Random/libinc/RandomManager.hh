#ifndef G4Random_RandomManager_hh
#define G4Random_RandomManager_hh

//Use this class to install a good random number generator and a given event
//generator in a geant4 job, and to make sure the seed is set and registered
//with the G4Interfaces globals at the start of each event. To use it properly
//you must:
//
// 1) Call RandomManager::init(seed) before G4 initialisation.
// 2) Call RandomManager::attach(pgen) before the first event is generated.
//
//Note that it is too late to do step 2) in an G4UserEventAction::BeginEvent as
//particle generation happens before.

#include "Core/Types.hh"
namespace G4Interfaces {
  class ParticleGenBase;
}

struct RandomManager {
  enum EVTMSGLEVEL { EVTMSG_NEVER, EVTMSG_ADAPTABLE, EVTMSG_ALWAYS };
  static void init(std::uint64_t seed_of_first_event, EVTMSGLEVEL lvl = EVTMSG_ADAPTABLE );
  static void attach(G4Interfaces::ParticleGenBase* the_particle_generator_of_the_job);
};

#endif
