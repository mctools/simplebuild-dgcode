#include "sbldpy/sbldpy.hh"
#include "G4Interfaces/ComponentHandles.hh"
#include "G4Interfaces/GeoBase.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Interfaces/StepFilterBase.hh"
#include <limits>

//Access to framework components created by sbldpy modules via their handles
//(used by G4Interfaces/python/ToPy.py).

namespace {
  using G4Interfaces::getComponent;
  using Handle = std::int64_t;

  std::string kind( Handle h )
  {
    auto& c = getComponent( h );
    if ( dynamic_cast<G4Interfaces::GeoBase*>( &c ) )
      return "geo";
    if ( dynamic_cast<G4Interfaces::ParticleGenBase*>( &c ) )
      return "gen";
    if ( dynamic_cast<G4Interfaces::StepFilterBase*>( &c ) )
      return "filter";
    return "other";
  }

  std::string get_name( Handle h )
  {
    auto& c = getComponent( h );
    if ( auto p = dynamic_cast<G4Interfaces::GeoBase*>( &c ) )
      return p->getName();
    if ( auto p = dynamic_cast<G4Interfaces::ParticleGenBase*>( &c ) )
      return p->getName();
    if ( auto p = dynamic_cast<G4Interfaces::StepFilterBase*>( &c ) )
      return p->getName();
    return "";
  }

  std::string join_list( const Utils::ParametersBase::ParameterList& l )
  {
    std::string res;
    for ( auto& e : l ) {
      if ( !res.empty() )
        res += '\n';
      res += e;
    }
    return res;
  }
}

SBLDPY_MODULE( _components )
{
  m.doc( "Access to framework components (geometries, generators and step filters)"
         " created by sbldpy modules, via their handles." );
  m.def( "kind", &kind, "Kind of component (geo, gen, filter or other)", { "handle" } );
  m.def( "get_name", &get_name, { "handle" } );
  m.def( "dump", []( Handle h, const std::string& prefix ) { getComponent( h ).dump( prefix.c_str() ); },
         { "handle", "prefix" } );
  m.def( "dump_nolock", []( Handle h, const std::string& prefix ) { getComponent( h ).dumpNoLock( prefix.c_str() ); },
         { "handle", "prefix" } );
  m.def( "lock", []( Handle h ) { getComponent( h ).lock(); }, { "handle" } );
  m.def( "is_locked", []( Handle h ) { return getComponent( h ).isLocked(); }, { "handle" } );
  m.def( "set_ignore_ranges", []( Handle h ) { getComponent( h ).setIgnoreRanges(); }, { "handle" } );
  m.def( "no_hard_exit_on_parameter_failure", []( Handle h ) { getComponent( h ).noHardExitOnParameterFailure(); },
         { "handle" } );
  m.def( "set_parameter_double", []( Handle h, const std::string& n, double v ) { getComponent( h ).setParameterDouble( n, v ); },
         { "handle", "name", "value" } );
  m.def( "set_parameter_int", []( Handle h, const std::string& n, std::int64_t v )
  {
    if ( v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max() )
      throw std::out_of_range( "Value out of range for integer parameter " + n );
    getComponent( h ).setParameterInt( n, static_cast<int>( v ) );
  }, { "handle", "name", "value" } );
  m.def( "set_parameter_boolean", []( Handle h, const std::string& n, bool v ) { getComponent( h ).setParameterBoolean( n, v ); },
         { "handle", "name", "value" } );
  m.def( "set_parameter_string", []( Handle h, const std::string& n, const std::string& v ) { getComponent( h ).setParameterString( n, v ); },
         { "handle", "name", "value" } );
  m.def( "get_parameter_double", []( Handle h, const std::string& n ) { return getComponent( h ).getParameterDoubleNoLock( n ); },
         { "handle", "name" } );
  m.def( "get_parameter_int", []( Handle h, const std::string& n ) { return getComponent( h ).getParameterIntNoLock( n ); },
         { "handle", "name" } );
  m.def( "get_parameter_boolean", []( Handle h, const std::string& n ) { return getComponent( h ).getParameterBooleanNoLock( n ); },
         { "handle", "name" } );
  m.def( "get_parameter_string", []( Handle h, const std::string& n ) { return std::string( getComponent( h ).getParameterStringCStrNoLock( n ) ); },
         { "handle", "name" } );
  m.def( "has_parameter_double", []( Handle h, const std::string& n ) { return getComponent( h ).hasParameterDouble( n ); },
         { "handle", "name" } );
  m.def( "has_parameter_int", []( Handle h, const std::string& n ) { return getComponent( h ).hasParameterInt( n ); },
         { "handle", "name" } );
  m.def( "has_parameter_boolean", []( Handle h, const std::string& n ) { return getComponent( h ).hasParameterBoolean( n ); },
         { "handle", "name" } );
  m.def( "has_parameter_string", []( Handle h, const std::string& n ) { return getComponent( h ).hasParameterString( n ); },
         { "handle", "name" } );
  //Parameter lists are returned as newline separated strings:
  m.def( "parameter_list_double", []( Handle h ) { Utils::ParametersBase::ParameterList l; getComponent( h ).getParameterListDouble( l ); return join_list( l ); },
         { "handle" } );
  m.def( "parameter_list_int", []( Handle h ) { Utils::ParametersBase::ParameterList l; getComponent( h ).getParameterListInt( l ); return join_list( l ); },
         { "handle" } );
  m.def( "parameter_list_boolean", []( Handle h ) { Utils::ParametersBase::ParameterList l; getComponent( h ).getParameterListBoolean( l ); return join_list( l ); },
         { "handle" } );
  m.def( "parameter_list_string", []( Handle h ) { Utils::ParametersBase::ParameterList l; getComponent( h ).getParameterListString( l ); return join_list( l ); },
         { "handle" } );
}
