#ifndef G4Interfaces_ComponentHandles_hh
#define G4Interfaces_ComponentHandles_hh

#include "Utils/ParametersBase.hh"
#include <cstdint>
#include <stdexcept>
#include <string>

namespace G4Interfaces {

  //Table of framework components (geometries, particle generators and step
  //filters) created by sbldpy based Python modules (see GeoConstructToPy.hh,
  //ParticleGenToPy.hh and StepFilterToPy.hh). On the Python side, components
  //are referred to by integer handles, so the component objects never have to
  //cross between Python modules. Like the components created by the older
  //pybind11 based modules, the components are never deleted.

  std::int64_t registerComponent( Utils::ParametersBase* );

  //Throws std::out_of_range for invalid handles:
  Utils::ParametersBase& getComponent( std::int64_t handle );

  //Throws std::invalid_argument if the component is not of the given type:
  template<class T>
  T& getComponentAs( std::int64_t handle, const char * what )
  {
    auto p = dynamic_cast<T*>( &getComponent( handle ) );
    if ( !p )
      throw std::invalid_argument( std::string("Component is not a ") + what );
    return *p;
  }

}

#endif
