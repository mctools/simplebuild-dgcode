#include "sbldpy/sbldpy.hh"
#include "G4Launcher/Launcher.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Interfaces/StepFilterBase.hh"
#include "G4Interfaces/ComponentHandles.hh"
#include "G4Interfaces/CppObjectHandles.hh"
#include "Utils/ParametersBase.hh"
#include "G4ThreeVector.hh"
#include "G4Version.hh"
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>

//Compiled part of the G4Launcher Python module (see G4Launcher/_launcher.py,
//which provides the actual Python interface). Since the launcher is a
//singleton, the functions here operate on the launcher returned by
//G4Launcher::Launcher::getTheLauncher(). Components are referred to by their
//handles in the table in G4Interfaces/ComponentHandles.hh, and user actions and
//hooks by their handles in the table in G4Interfaces/CppObjectHandles.hh.

namespace G4Launcher_py {

  using Launcher = G4Launcher::Launcher;
  using Handle = std::int64_t;
  using Vect = std::tuple<double,double,double>;

  Launcher& launcher()
  {
    auto l = Launcher::getTheLauncher();
    if ( !l )
      throw std::runtime_error("G4Launcher not created");
    return *l;
  }

  unsigned toUnsigned( std::int64_t n )
  {
    if ( n < 0 || n > std::numeric_limits<unsigned>::max() )
      throw std::out_of_range("number out of range");
    return static_cast<unsigned>( n );
  }

  std::string str( const char * s )
  {
    return s ? std::string( s ) : std::string();
  }

  G4ThreeVector toG4Vect( const Vect& v )
  {
    return G4ThreeVector( std::get<0>(v), std::get<1>(v), std::get<2>(v) );
  }

  template <class T>
  T* component( Handle h, const char * fctname )
  {
    auto p = dynamic_cast<T*>( &G4Interfaces::getComponent( h ) );
    if ( !p )
      throw std::runtime_error(std::string("Object of invalid type passed to .")+fctname+"(..)");
    return p;
  }

  void create()
  {
    new Launcher;//Never deleted, due to singleton nature of G4Launcher (released by shutdown).
  }

  //Python callables as hooks:
  class PyHook : public G4Interfaces::HookFct {
  public:
    PyHook( sbldpy::Callback<void()> cb ) : m_cb( std::move( cb ) ) {}
    void operator()() override { m_cb(); }
  private:
    sbldpy::Callback<void()> m_cb;
  };

  using AddHookFct = void (Launcher::*)( Launcher::HookFctPtr );
  template <AddHookFct f>
  void addPyHook( sbldpy::Callback<void()> cb ) { ( launcher().*f )( std::make_shared<PyHook>( std::move( cb ) ) ); }
  template <AddHookFct f>
  void addObjHook( Handle h ) { ( launcher().*f )( G4Interfaces::claimHookFct( h ) ); }

  //Extra parameters defined on the launcher from Python:
  class LauncherExtraPars : public Utils::ParametersBase {
  public:
    using Utils::ParametersBase::addParameterDouble;
    using Utils::ParametersBase::addParameterInt;
    using Utils::ParametersBase::addParameterBoolean;
    using Utils::ParametersBase::addParameterString;
  };

  LauncherExtraPars& extraPars( Handle h )
  {
    return G4Interfaces::getComponentAs<LauncherExtraPars>( h, "LauncherExtraPars" );
  }

  int toInt( std::int64_t n )
  {
    if ( n < std::numeric_limits<int>::min() || n > std::numeric_limits<int>::max() )
      throw std::out_of_range("integer out of range");
    return static_cast<int>( n );
  }

}

SBLDPY_MODULE( _impl )
{
  using namespace G4Launcher_py;
  using G4Launcher_py::Launcher;

  m.def( "g4version", []() -> std::int64_t { return G4VERSION_NUMBER; } );
  m.def( "g4versionstr", []() { return std::string( G4Version ); } );

  m.def( "create", &create );
  m.def( "exists", []() { return Launcher::getTheLauncher() != nullptr; } );
  m.def( "setGeo", []( Handle h ) { launcher().setGeo( component<G4Interfaces::GeoConstructBase>( h, "setGeo" ) ); } );
  m.def( "setGen", []( Handle h ) { launcher().setGen( component<G4Interfaces::ParticleGenBase>( h, "setGen" ) ); } );
  m.def( "setFilter", []( Handle h ) { launcher().setFilter( component<G4Interfaces::StepFilterBase>( h, "setFilter" ) ); } );
  m.def( "setKillFilter", []( Handle h ) { launcher().setKillFilter( component<G4Interfaces::StepFilterBase>( h, "setKillFilter" ) ); } );
  m.def( "setMultiProcessing", []( std::int64_t n ) { launcher().setMultiProcessing( toUnsigned( n ) ); } );
  m.def( "setVis", []( const std::string& e ) { launcher().setVis( e.c_str() ); } );
  m.def( "startSession", []() { launcher().startSession(); } );
  m.def( "startSimulation", []( std::int64_t n ) { launcher().startSimulation( toUnsigned( n ) ); } );
  m.def( "setRndEvtMsgMode", []( const std::string& mode ) { launcher().setRndEvtMsgMode( mode.c_str() ); } );
  m.def( "rndEvtMsgMode", []() { return launcher().rndEvtMsgMode(); } );
  m.def( "setPhysicsList", []( const std::string& pl ) { launcher().setPhysicsList( pl.c_str() ); } );
  m.def( "hasPhysicsListProvider", []() { return launcher().hasPhysicsListProvider(); } );
  m.def( "setParticleGunPDG", []( std::int64_t pdg, double ekin, Vect pos, Vect dir )
  {
    launcher().setParticleGun( toInt( pdg ), ekin, toG4Vect( pos ), toG4Vect( dir ) );
  } );
  m.def( "setParticleGunName", []( const std::string& name, double ekin, Vect pos, Vect dir )
  {
    launcher().setParticleGun( name.c_str(), ekin, toG4Vect( pos ), toG4Vect( dir ) );
  } );
  m.def( "setOutput", []( const std::string& fn, const std::string& mode ) { launcher().setOutput( fn.c_str(), mode.c_str() ); } );
  m.def( "closeOutput", []() { launcher().closeOutput(); } );
  m.def( "noRandomSetup", []() { launcher().noRandomSetup(); } );
  //Seeds are passed as two 32 bit numbers, to support the full range of 64 bit
  //unsigned integers:
  m.def( "setSeed", []( std::int64_t hi, std::int64_t lo )
  {
    const std::int64_t max32 = 0xFFFFFFFF;
    if ( hi < 0 || hi > max32 || lo < 0 || lo > max32 )
      throw std::out_of_range("invalid seed");
    launcher().setSeed( ( static_cast<std::uint64_t>( hi ) << 32 ) | static_cast<std::uint64_t>( lo ) );
  } );
  m.def( "getSeed", []() { return launcher().getSeed(); } );
  m.def( "setUserSteppingAction", []( Handle h ) { launcher().setUserSteppingAction( G4Interfaces::claimUserSteppingAction( h ) ); } );
  m.def( "setUserEventAction", []( Handle h ) { launcher().setUserEventAction( G4Interfaces::claimUserEventAction( h ) ); } );
  m.def( "addPreGenHook", []( Handle h ) { launcher().addPreGenHook( G4Interfaces::claimPreGenHook( h ) ); } );
  m.def( "addPostGenHook", []( Handle h ) { launcher().addPostGenHook( G4Interfaces::claimPostGenHook( h ) ); } );
  m.def( "cmd", []( const std::string& c ) { launcher().cmd( c.c_str() ); } );
  m.def( "cmd_preinit", []( const std::string& c ) { launcher().cmd_preinit( c.c_str() ); } );
  m.def( "cmd_postinit", []( const std::string& c ) { launcher().cmd_postinit( c.c_str() ); } );
  m.def( "init", []() { launcher().init(); } );
  m.def( "initVis", []() { launcher().initVis(); } );
  m.def( "getMultiProcessing", []() { return launcher().getMultiProcessing(); } );
  m.def( "GetNoRandomSetup", []() { return launcher().GetNoRandomSetup(); } );
  m.def( "getOutputFile", []() { return str( launcher().getOutputFile() ); } );
  m.def( "getOutputMode", []() { return str( launcher().getOutputMode() ); } );
  m.def( "getVis", []() { return str( launcher().getVis() ); } );
  m.def( "getPhysicsList", []() { return str( launcher().getPhysicsList() ); } );
  m.def( "getPrintPrefix", []() { return str( launcher().getPrintPrefix() ); } );
  m.def( "allowMultipleSettings", []() { launcher().allowMultipleSettings(); } );
  m.def( "addHist", []( const std::string& c ) { launcher().addHist( c.c_str() ); } );
  m.def( "setUserData", []( const std::string& k, const std::string& v ) { launcher().setUserData( k.c_str(), v.c_str() ); } );
  m.def( "allowFPE", []() { launcher().allowFPE(); } );
  m.def( "dumpGDML", []( const std::string& fn ) { launcher().dumpGDML( fn.c_str() ); } );
  m.def( "preInitDone", []() { return launcher().preInitDone(); } );
  m.def( "rmIsInit", []() { return launcher().rmIsInit(); } );
  m.def( "addPrePreInitHook", &addPyHook<&Launcher::addPrePreInitHook> );
  m.def( "addPreInitHook", &addPyHook<&Launcher::addPreInitHook> );
  m.def( "addPostInitHook", &addPyHook<&Launcher::addPostInitHook> );
  m.def( "addPostSimHook", &addPyHook<&Launcher::addPostSimHook> );
  m.def( "addPostMPHook", &addPyHook<&Launcher::addPostMPHook> );
  m.def( "addPrePreInitHookObj", &addObjHook<&Launcher::addPrePreInitHook> );
  m.def( "addPreInitHookObj", &addObjHook<&Launcher::addPreInitHook> );
  m.def( "addPostInitHookObj", &addObjHook<&Launcher::addPostInitHook> );
  m.def( "addPostSimHookObj", &addObjHook<&Launcher::addPostSimHook> );
  m.def( "addPostMPHookObj", &addObjHook<&Launcher::addPostMPHook> );
  m.def( "shutdown", []() { launcher().shutdown(); } );

  m.def( "extrapars_create", []() { return G4Interfaces::registerComponent( new LauncherExtraPars ); } );
  m.def( "extrapars_add_double", []( Handle h, const std::string& n, double v ) { extraPars( h ).addParameterDouble( n, v ); } );
  m.def( "extrapars_add_int", []( Handle h, const std::string& n, std::int64_t v ) { extraPars( h ).addParameterInt( n, toInt( v ) ); } );
  m.def( "extrapars_add_boolean", []( Handle h, const std::string& n, bool v ) { extraPars( h ).addParameterBoolean( n, v ); } );
  m.def( "extrapars_add_string", []( Handle h, const std::string& n, const std::string& v ) { extraPars( h ).addParameterString( n, v ); } );
}
