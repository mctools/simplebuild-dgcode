#ifndef G4PhysicsLists_PhysicsListFactory_hh
#define G4PhysicsLists_PhysicsListFactory_hh

#include <vector>
#include <string>
class G4VUserPhysicsList;

//This is a physics list factory which can be used to detect available G4
//reference physics lists and to instantiate them.
//
//NOTE: Custom "home-made" physics lists should no longer be added here, they
//will be detected and instantiated on the python-side.

class PhysicsListFactory {
public:
  static G4VUserPhysicsList * createReferencePhysicsList(const std::string& name);
  static void getAllReferenceListNames(std::vector<std::string>&);
  static G4VUserPhysicsList * attemptCreateCustomPhysicsList(const std::string& name);
};

#endif
