#include "G4PhysicsLists/PhysicsListFactory.hh"
#include "G4PhysListFactory.hh"
#include <cstdlib>

G4VUserPhysicsList * PhysicsListFactory::createReferencePhysicsList(const std::string& name)
{
  //G4 provided reference lists:
  G4PhysListFactory reflist_factory;
  return reflist_factory.GetReferencePhysList(name);
}

void PhysicsListFactory::getAllReferenceListNames(std::vector<std::string>& v)
{
  v.clear();
  //G4 provided reference lists:
  G4PhysListFactory reflist_factory;
  auto refpl_buggy = reflist_factory.AvailablePhysLists();
  auto refplEM = reflist_factory.AvailablePhysListsEM();//alternative endings for EM physics.

  //Workaround bug in G4PhysListFactory: QGSP is too much, ShieldingLEND is
  //missing. Also, ShieldingLEND should only be there when G4LENDDATA is set.
  const char * envG4LENDDATA_cstr = getenv("G4LENDDATA");
  std::string envG4LENDDATA(envG4LENDDATA_cstr?envG4LENDDATA_cstr:"");
  bool allowLEND = !envG4LENDDATA.empty();

  std::vector<std::string> refpl;
  bool sawShieldingLEND = false;
  for (auto it = refpl_buggy.begin();it != refpl_buggy.end();++it) {
    if (*it=="QGSP")
      continue;
    if (*it=="ShieldingLEND") {
      sawShieldingLEND = true;
      if (allowLEND)
        refpl.push_back(*it);
    } else {
      refpl.push_back(*it);
    }
  }
  if (allowLEND&&!sawShieldingLEND)
    refpl.push_back("ShieldingLEND");

  //combine hadronic and em parts (a large number admittedly):
  for (auto it = refpl.begin();it != refpl.end();++it) {
    for (auto itEM = refplEM.begin();itEM!= refplEM.end();++itEM)
      v.push_back(*it+*itEM);
  }
}
