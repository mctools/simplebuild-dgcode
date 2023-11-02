#ifndef G4Materials_NamedMaterialProvider_hh
#define G4Materials_NamedMaterialProvider_hh

//Interface to material creation which makes sure that all materials can be
//defined at configuration time by a simple string. The general format is
//"NAME:par1=val:par2=val2:...".
//
//More info at: https://confluence.esss.lu.se/display/DG/NamedMaterials

#include <string>
class G4Material;

namespace NamedMaterialProvider {

  //Access materials:
  G4Material * getMaterial(const std::string&);

  //Normally this will be called by the framework:
  void setPrintPrefix(const char*);
}
#endif
