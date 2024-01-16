#ifndef G4PhysicsLists_PhysicsListEmpty_hh
#define G4PhysicsLists_PhysicsListEmpty_hh

#include "G4VModularPhysicsList.hh"

//Fake empty physics list (useful for e.g. geometry tracing)

class PhysicsListEmpty : public G4VModularPhysicsList
{
public:
  PhysicsListEmpty();
  virtual ~PhysicsListEmpty();
  virtual void ConstructParticle();
  virtual void ConstructProcess();
};

#endif
