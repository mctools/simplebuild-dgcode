#include "G4PhysicsLists/PhysicsListEmpty.hh"
#include "G4BaryonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4Gamma.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"
#include "G4ShortLivedConstructor.hh"

PhysicsListEmpty :: PhysicsListEmpty()
 : G4VModularPhysicsList()
{
}

PhysicsListEmpty::~PhysicsListEmpty()
{
}

void PhysicsListEmpty::ConstructParticle()
{
  G4BaryonConstructor::ConstructParticle();
  G4MesonConstructor::ConstructParticle();
  G4LeptonConstructor::ConstructParticle();
  G4IonConstructor::ConstructParticle();
  G4ShortLivedConstructor::ConstructParticle();
  G4Gamma::GammaDefinition();
  G4ChargedGeantino::ChargedGeantinoDefinition();
  G4Geantino::GeantinoDefinition();
}

void PhysicsListEmpty::ConstructProcess()
{
  AddTransportation();
}



extern "C" {
  G4VUserPhysicsList * sbldplugindef_g4physlist_Empty()
  {
    return new PhysicsListEmpty;
  }
}
