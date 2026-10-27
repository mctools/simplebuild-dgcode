#ifndef G4Interfaces_GeoConstructToPy_hh
#define G4Interfaces_GeoConstructToPy_hh

#ifndef SBLDPY_MODNAME
#  error "The GeoConstructToPy.hh header can only be used in pymod_* folders."
#endif

#include "sbldpy/sbldpy.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "G4Interfaces/ComponentToPy.hh"
#include <type_traits>

//For easier migration of code from the corresponding *PyExport.hh headers
//used in pycpp_* folders, we include the same Geant4 headers as those (and
//cassert, which used to be included indirectly):
#include "G4VPhysicalVolume.hh"
#include <cassert>

namespace G4Interfaces {

  //Defines the function create_handle() in an sbldpy module, which creates a
  //geometry of the given class and returns its handle. The Python side wraps
  //the handle in a component object (see G4Interfaces/python/ToPy.py).
  //Usage:
  //
  //  SBLDPY_MODULE( _MyClass ) { G4Interfaces::geoToPy<MyClass>( m ); }
  //
  //Extra methods can be added to the Python objects via the returned helper
  //object (see G4Interfaces/ComponentToPy.hh).

  template <class T>
  ComponentToPy<T> geoToPy( sbldpy::Module& mod )
  {
    static_assert( std::is_base_of_v<GeoConstructBase,T>, "Class must derive from G4Interfaces::GeoConstructBase" );
    mod.def( "create_handle", []() { return registerComponent( new T ); },
           "Create a new geometry and return its handle" );
    return ComponentToPy<T>( mod, "geometry" );
  }

}

#endif
