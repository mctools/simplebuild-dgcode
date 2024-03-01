#ifndef dgcode_PhysListMgr_hh
#define dgcode_PhysListMgr_hh

#include <string>
#include <vector>
class G4VUserPhysicsList;

namespace PhysListMgr {
  struct PhysListInfo {
    std::string name;
    std::string pkg_name;//not set for g4 ref lists
    std::string description;//only set if loaded.
  };
  enum class LoadDescriptions { YES, NO };
  std::vector<PhysListInfo> getAllLists( LoadDescriptions
                                         = LoadDescriptions::NO );
  G4VUserPhysicsList * createList( std::string name );
}

#endif
