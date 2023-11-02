#ifndef G4Interfaces_GeoConstructPyExport_hh
#define G4Interfaces_GeoConstructPyExport_hh

#ifndef PYMODNAME
#  error "Do not include the GeoConstructPyExport.hh header for code outside pycpp_* folders."
#endif

#include "Core/Python.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "G4VPhysicalVolume.hh"

namespace GeoConstructPyExport {

  template <class T>
  T* _internal_create()
  {
    return new T;
  }

  template <class T>
  py::class_<T,G4Interfaces::GeoConstructBase> exportGeo(py::module_ themod, const char* name)
  {
    pyextra::pyimport("G4Interfaces");
    //Fixme: std::shared_ptr instead?
    themod.def("create",&_internal_create<T>,py::return_value_policy::reference);
    return py::class_<T,G4Interfaces::GeoConstructBase>(themod,name)
      .def("Construct",&T::Construct,py::return_value_policy::reference);
  }


}

#endif
