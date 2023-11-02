#ifndef G4Materials_ShieldingMaterials_hh
#define G4Materials_ShieldingMaterials_hh

//Material factory for custom materials needed mainly by the shielding group.

#include <string>
#include <vector>
class G4Material;

namespace ShieldingMaterials {

  void getMatNames(std::vector<std::string>&);
  G4Material * createMat(const std::string& name);

}

#endif
