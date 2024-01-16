#ifndef G4Utils_FindMaterials_hh
#define G4Utils_FindMaterials_hh

class G4Material;
#include <set>

namespace G4Utils {

  void getAllMaterialsUsedByGeometry(std::set<G4Material*>& s);

}
#endif
