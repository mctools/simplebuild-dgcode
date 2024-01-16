#ifndef G4Interfaces_StepFilterPyExport_hh
#define G4Interfaces_StepFilterPyExport_hh

#ifndef PYMODNAME
#  error "Do not include the StepFilterPyExport.hh header for code outside pycpp_* folders."
#endif

#include "Core/Python.hh"
#include "G4Interfaces/StepFilterBase.hh"
#include "G4Step.hh"

namespace StepFilterPyExport {

  template <class T>
  T* _internal_create()
  {
    return new T;
  }

  template <class T>
  void exportFilter(py::module_ themod,const char* name)
  {
    pyextra::pyimport("G4Interfaces");
    themod.def("create",&_internal_create<T>,py::return_value_policy::reference);
    py::class_<T,G4Interfaces::StepFilterBase>(themod,name)
      ;
  }

}

#endif
