#ifndef G4Interfaces_StepFilterToPy_hh
#define G4Interfaces_StepFilterToPy_hh

#ifndef SBLDPY_MODNAME
#  error "The StepFilterToPy.hh header can only be used in pymod_* folders."
#endif

#include "sbldpy/sbldpy.hh"
#include "G4Interfaces/StepFilterBase.hh"
#include "G4Interfaces/ComponentToPy.hh"
#include <type_traits>

//For easier migration of code from the corresponding *PyExport.hh headers
//used in pycpp_* folders, we include the same Geant4 headers as those (and
//cassert, which used to be included indirectly):
#include "G4Step.hh"
#include <cassert>

namespace G4Interfaces {

  //Defines the function create_handle() in an sbldpy module, which creates a
  //step filter of the given class and returns its handle. The Python side wraps
  //the handle in a component object (see G4Interfaces/python/ToPy.py).
  //Usage:
  //
  //  SBLDPY_MODULE( _MyClass ) { G4Interfaces::filterToPy<MyClass>( m ); }
  //
  //Extra methods can be added to the Python objects via the returned helper
  //object (see G4Interfaces/ComponentToPy.hh).

  template <class T>
  ComponentToPy<T> filterToPy( sbldpy::Module& mod )
  {
    static_assert( std::is_base_of_v<StepFilterBase,T>, "Class must derive from G4Interfaces::StepFilterBase" );
    mod.def( "create_handle", []() { return registerComponent( new T ); },
           "Create a new step filter and return its handle" );
    return ComponentToPy<T>( mod, "step filter" );
  }

}

#endif
