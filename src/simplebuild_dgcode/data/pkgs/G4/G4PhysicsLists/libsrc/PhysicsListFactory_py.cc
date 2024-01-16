#include "Core/Python.hh"
#include "G4PhysicsLists/PhysicsListFactory.hh"
#include "G4Interfaces/PhysListProviderBase.hh"
#include "G4VUserPhysicsList.hh"

G4VUserPhysicsList * PhysicsListFactory::attemptCreateCustomPhysicsList(const std::string& name)
{
  pyextra::ensurePyInit();
  auto mod = pyextra::pyimport("G4PhysicsLists");
  auto pyp = mod.attr("extractProvider")(name);
  if (!pyp)
    return nullptr;
  G4Interfaces::PhysListProviderBase* p = pyp.cast<G4Interfaces::PhysListProviderBase*>();
  assert(p);
  return p->construct();
}

