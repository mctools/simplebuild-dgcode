#ifndef G4Interfaces_PhysicsListPyExport_hh
#define G4Interfaces_PhysicsListPyExport_hh

#ifndef PYMODNAME
#  error "Do not include the PhysicsListPyExport.hh header for code outside pycpp_* folders."
#endif

#include "Core/Python.hh"
#include <cassert>
#include "Core/String.hh"
#include "G4Interfaces/PhysListProviderBase.hh"

namespace PhysicsListPyExport {

  template <class T>
  class Provider final : public G4Interfaces::PhysListProviderBase {
  public:
    Provider(const char*name) : G4Interfaces::PhysListProviderBase(name) {}
    virtual ~Provider(){}
    G4VUserPhysicsList * construct() override {
      return new T;
    }
  };

  template <class T>
  Provider<T>* _internal_create_provider(const char* provider_name)
  {
    return new Provider<T>(provider_name);
  }

  //Not exposing base directly like for gen/geo (could do if needed of course).
  //Also, we are skipping the name parameter => one physics list per module.
  //The reason being that we want to be able to identify available physics lists
  //merely from looking at module names.

  template <class T>
  py::class_<T> exportPhysList( py::module_ themod )
  {
    //remember, if not using gcc, __GNUCC__ will evaluate to 0 (we want to exclude gcc <= 4.5)
    if ( strncmp("g4physlist_",sbld_stringify(PYMODNAME),11)!=0 )
      throw std::runtime_error("ERROR: Names of python modules containing G4 physics lists must all be prefixed with g4physlist_");
    std::string bstr = sbld_stringify(PYMODNAME);
    const char * class_name = &bstr[11];

    pyextra::pyimport("G4Interfaces");

    themod.def("create_provider",&_internal_create_provider<T>,py::return_value_policy::reference);

    py::class_<Provider<T>,G4Interfaces::PhysListProviderBase >(themod,(std::string("Provider__")+class_name).c_str());
    return py::class_<T>(themod,class_name);
  }
}

#endif
