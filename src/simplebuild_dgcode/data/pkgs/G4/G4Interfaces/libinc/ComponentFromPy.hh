#ifndef G4Interfaces_ComponentFromPy_hh
#define G4Interfaces_ComponentFromPy_hh

#ifndef PYMODNAME
#  error "Do not include the ComponentFromPy.hh header for code outside pycpp_* folders."
#endif

#include "Core/Python.hh"
#include "G4Interfaces/ComponentHandles.hh"
#include <cstdint>

namespace G4Interfaces {

  //Get the C++ object of a framework component (geometry, particle generator
  //or step filter) passed from Python to pybind11 based code. This supports
  //both components from pybind11 based modules and components from sbldpy
  //based modules (see G4Interfaces/GeoConstructToPy.hh), which are Python
  //objects with a _g4handle attribute referring to the component in the table
  //in G4Interfaces/ComponentHandles.hh. Returns nullptr if the object is not a
  //component of the requested type. Example:
  //
  //  auto geo = G4Interfaces::componentFromPy<G4Interfaces::GeoConstructBase>( o );

  template <class T>
  T* componentFromPy( py::handle o )
  {
    if ( py::hasattr( o, "_g4handle" ) ) {
      auto& c = getComponent( py::cast<std::int64_t>( o.attr("_g4handle") ) );
      return dynamic_cast<T*>( &c );
    }
    if ( py::isinstance<T>( o ) )
      return py::cast<T*>( o );
    return nullptr;
  }

}

#endif
