#ifndef G4Interfaces_ParticleGenToPy_hh
#define G4Interfaces_ParticleGenToPy_hh

#ifndef SBLDPY_MODNAME
#  error "The ParticleGenToPy.hh header can only be used in pymod_* folders."
#endif

#include "sbldpy/sbldpy.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Interfaces/ComponentHandles.hh"
#include <type_traits>

//For easier migration of code from the corresponding *PyExport.hh headers
//used in pycpp_* folders, we include the same Geant4 headers as those (and
//cassert, which used to be included indirectly):
#include "G4Event.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include <cassert>

namespace G4Interfaces {

  //Defines the function create_handle() in an sbldpy module, which creates a
  //particle generator of the given class and returns its handle. The Python side wraps
  //the handle in a component object (see G4Interfaces/python/ToPy.py).
  //Usage:
  //
  //  SBLDPY_MODULE( _MyClass ) { G4Interfaces::genToPy<MyClass>( m ); }

  template <class T>
  void genToPy( sbldpy::Module& m )
  {
    static_assert( std::is_base_of_v<ParticleGenBase,T>, "Class must derive from G4Interfaces::ParticleGenBase" );
    m.def( "create_handle", []() { return registerComponent( new T ); },
           "Create a new particle generator and return its handle" );
  }

}

#endif
