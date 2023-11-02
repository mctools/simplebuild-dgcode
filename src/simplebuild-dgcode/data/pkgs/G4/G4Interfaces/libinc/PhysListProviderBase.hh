#ifndef G4Interfaces_PhysListProviderBase_hh
#define G4Interfaces_PhysListProviderBase_hh

#include "G4VUserPhysicsList.hh"
#include <string>

namespace G4Interfaces {

  //This is used to hook into the G4Launcher/Launcher in order to create a named
  //physics list via a call-back method. It is envisioned to be used mainly by
  //the python-side framework to provide physics list through python modules
  //(i.e. without compile-time dependencies).

  class PhysListProviderBase
  {
  public:
    //Derived classes must:
    //
    //  1) Provide a name with  : PhysListProviderBase("some name")
    //  2) Be ready to construct and return a physics list in the construct method.

    PhysListProviderBase(const char* name) : m_name(name) {}
    virtual ~PhysListProviderBase(){}
    const char* getName() const { return m_name.c_str(); }
    virtual G4VUserPhysicsList * construct() = 0;

  private:
    std::string m_name;
  };

}

#endif
