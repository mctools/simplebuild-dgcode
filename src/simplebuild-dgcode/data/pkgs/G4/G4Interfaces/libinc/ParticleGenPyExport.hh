#ifndef G4Interfaces_ParticleGenPyExport_hh
#define G4Interfaces_ParticleGenPyExport_hh

#ifndef PYMODNAME
#  error "Do not include the ParticleGenPyExport.hh header for code outside pycpp_* folders."
#endif

#include "Core/Python.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Event.hh"

#include "G4VUserPrimaryGeneratorAction.hh"

namespace ParticleGenPyExport {

  template <class T>
  T* _internal_create()
  {
    return new T;
  }

  template <class T>
  py::class_<T,G4Interfaces::ParticleGenBase> exportGen(py::module_ themod, const char* name)
  {
    pyextra::pyimport("G4Interfaces");
    themod.def("create",&_internal_create<T>,py::return_value_policy::reference);
    return py::class_<T,G4Interfaces::ParticleGenBase>(themod,name)
      .def("getAction",&T::getAction,py::return_value_policy::reference);
  }
}

#endif
