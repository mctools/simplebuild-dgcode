#include "sbldpy/sbldpy.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Interfaces/ComponentHandles.hh"
#include "G4Utils/GenUtils.hh"
#include "Utils/NeutronMath.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4Event.hh"
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

//Compiled part of G4CustomPyGen (see python/__init__.py, which provides the
//GenBase class and the gun). To keep the overhead low, there is only one call
//into Python per event: The Python generate_event method records the particles
//fired with the gun, and they are passed back to C++ as a flat array with
//NPARTFIELDS numbers per particle (see the fields below), from which the
//primary vertices are created.

namespace G4CustomPyGen_py {

  using Handle = std::int64_t;
  using Vect = std::tuple<double,double,double>;

  //Fields of each particle in the array returned by generate_event:
  enum ParticleField { F_PARTICLE, F_ENERGY, F_X, F_Y, F_Z, F_DX, F_DY, F_DZ,
                       F_TIME, F_WEIGHT, NPARTFIELDS };
  //F_PARTICLE is an index in the particle definitions table below. F_ENERGY is
  //the kinetic energy, or minus the wavelength if the latter was set (the
  //conversion is done here, to give exactly the same results as before).

  //Particle definitions referred to by indices on the Python side (since some
  //particles, like geantinos, have no unique PDG code):
  std::vector<G4ParticleDefinition*>& particleDefs()
  {
    static std::vector<G4ParticleDefinition*> defs;
    return defs;
  }

  std::int64_t addParticleDef( G4ParticleDefinition* def )
  {
    auto& defs = particleDefs();
    for ( std::size_t i = 0; i < defs.size(); ++i )
      if ( defs[i] == def )
        return static_cast<std::int64_t>( i );
    defs.push_back( def );
    return static_cast<std::int64_t>( defs.size() - 1 );
  }

  //Returns the index and the PDG code:
  std::tuple<std::int64_t,std::int64_t> particleByName( const std::string& name )
  {
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(name);
    if (!particle) {
      printf("G4CustomPyGen set_type ERROR: Could not find particle with name %s\n",name.c_str());
      throw std::runtime_error("G4CustomPyGen set_type ERROR: Could not find particle");
    }
    return { addParticleDef( particle ), particle->GetPDGEncoding() };
  }

  std::int64_t particleByPDG( std::int64_t pdg64 )
  {
    if ( pdg64 < INT32_MIN || pdg64 > INT32_MAX )
      throw std::out_of_range("pdg code out of range");
    const int pdg_code = static_cast<int>( pdg64 );
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(pdg_code);
    if ( !particle && (pdg_code/100000000 == 10)) {
      //Not in ParticleTable and pdgcode is of form 10xxxxxxxx, so look for ion:
      particle = G4IonTable::GetIonTable()->GetIon(pdg_code);
    }
    if (!particle) {
      printf("G4CustomPyGen set_type ERROR: Could not find particle with pdg code %i\n",pdg_code);
      throw std::runtime_error("G4CustomPyGen set_type ERROR: Could not find particle");
    }
    return addParticleDef( particle );
  }

  class PyGen : public G4Interfaces::ParticleGenBase {
  public:
    PyGen() : ParticleGenBase("_tmpname_") {}//Name is set on the Python side

    void setCallbacks( sbldpy::Callback<bool()> validate,
                       sbldpy::Callback<void()> initgen,
                       sbldpy::Callback<std::vector<double>()> genevt )
    {
      m_validate = std::make_unique<sbldpy::Callback<bool()>>( std::move( validate ) );
      m_initgen = std::make_unique<sbldpy::Callback<void()>>( std::move( initgen ) );
      m_genevt = std::make_unique<sbldpy::Callback<std::vector<double>()>>( std::move( genevt ) );
    }

    void init() override
    {
      if ( !m_genevt )
        throw std::runtime_error("G4CustomPyGen: generator not set up from Python");
      m_gun = std::make_unique<G4ParticleGun>( 1 );
      (*m_initgen)();
    }

    void gen( G4Event* evt ) override
    {
      const std::vector<double> parts = (*m_genevt)();
      if ( parts.size() % NPARTFIELDS )
        throw std::runtime_error("G4CustomPyGen: invalid particle data");
      const auto& defs = particleDefs();
      for ( std::size_t i = 0; i < parts.size(); i += NPARTFIELDS ) {
        const double * p = &parts[i];
        const auto idx = static_cast<std::size_t>( p[F_PARTICLE] );
        if ( !( p[F_PARTICLE] >= 0.0 ) || idx >= defs.size() )
          throw std::runtime_error("G4CustomPyGen: invalid particle data");
        if ( m_gun->GetParticleDefinition() != defs[idx] )
          m_gun->SetParticleDefinition( defs[idx] );
        m_gun->SetParticleEnergy( p[F_ENERGY] < 0.0
                                  ? Utils::neutronWavelengthToEKin( -p[F_ENERGY] )
                                  : p[F_ENERGY] );
        m_gun->SetParticlePosition( G4ThreeVector( p[F_X], p[F_Y], p[F_Z] ) );
        m_gun->SetParticleMomentumDirection( G4ThreeVector( p[F_DX], p[F_DY], p[F_DZ] ) );
        m_gun->SetParticleTime( p[F_TIME] );
        const int iv = evt->GetNumberOfPrimaryVertex();
        m_gun->GeneratePrimaryVertex( evt );
        if ( p[F_WEIGHT] != 1.0 )
          evt->GetPrimaryVertex( iv )->SetWeight( p[F_WEIGHT] );
      }
    }

    bool unlimited() const override { return m_unlimited; }
    void setUnlimited( bool u ) { m_unlimited = u; }

    //Expose protected methods:
    using ParticleGenBase::rand;
    using ParticleGenBase::randGauss;
    using ParticleGenBase::randExponential;
    using ParticleGenBase::randPoisson;
    using ParticleGenBase::randInt;
    using ParticleGenBase::setName;
    using ParticleGenBase::signalEndOfEvents;
    using ParticleGenBase::addParameterDouble;
    using ParticleGenBase::addParameterInt;
    using ParticleGenBase::addParameterBoolean;
    using ParticleGenBase::addParameterString;

  protected:
    bool validateParameters() override
    {
      return m_validate ? (*m_validate)() : true;
    }

  private:
    std::unique_ptr<sbldpy::Callback<bool()>> m_validate;
    std::unique_ptr<sbldpy::Callback<void()>> m_initgen;
    std::unique_ptr<sbldpy::Callback<std::vector<double>()>> m_genevt;
    std::unique_ptr<G4ParticleGun> m_gun;
    bool m_unlimited = true;
  };

  PyGen& pygen( Handle h )
  {
    return G4Interfaces::getComponentAs<PyGen>( h, "G4CustomPyGen generator" );
  }

  int toInt( std::int64_t n )
  {
    if ( n < INT32_MIN || n > INT32_MAX )
      throw std::out_of_range("integer out of range");
    return static_cast<int>( n );
  }

  long toLong( std::int64_t n )
  {
    if ( n < std::numeric_limits<long>::min() || n > std::numeric_limits<long>::max() )
      throw std::out_of_range("integer out of range");
    return static_cast<long>( n );
  }

}

SBLDPY_MODULE( _impl )
{
  using namespace G4CustomPyGen_py;
  m.def( "create_handle", []() { return G4Interfaces::registerComponent( new PyGen ); } );
  m.def( "set_callbacks", []( Handle h, sbldpy::Callback<bool()> validate, sbldpy::Callback<void()> initgen,
                              sbldpy::Callback<std::vector<double>()> genevt )
  {
    pygen( h ).setCallbacks( std::move( validate ), std::move( initgen ), std::move( genevt ) );
  } );
  m.def( "set_name", []( Handle h, const std::string& n ) { pygen( h ).setName( n.c_str() ); } );
  m.def( "set_unlimited", []( Handle h, bool u ) { pygen( h ).setUnlimited( u ); } );
  m.def( "signal_end_of_events", []( Handle h, bool incl ) { pygen( h ).signalEndOfEvents( incl ); } );
  m.def( "add_parameter_double", []( Handle h, const std::string& n, double v ) { pygen( h ).addParameterDouble( n, v ); } );
  m.def( "add_parameter_double_range", []( Handle h, const std::string& n, double v, double lo, double hi )
  {
    pygen( h ).addParameterDouble( n, v, lo, hi );
  } );
  m.def( "add_parameter_int", []( Handle h, const std::string& n, std::int64_t v ) { pygen( h ).addParameterInt( n, toInt( v ) ); } );
  m.def( "add_parameter_int_range", []( Handle h, const std::string& n, std::int64_t v, std::int64_t lo, std::int64_t hi )
  {
    pygen( h ).addParameterInt( n, toInt( v ), toInt( lo ), toInt( hi ) );
  } );
  m.def( "add_parameter_boolean", []( Handle h, const std::string& n, bool v ) { pygen( h ).addParameterBoolean( n, v ); } );
  m.def( "add_parameter_string", []( Handle h, const std::string& n, const std::string& v ) { pygen( h ).addParameterString( n, v ); } );
  m.def( "rand", []( Handle h ) { return pygen( h ).rand(); } );
  m.def( "rand_range", []( Handle h, double a, double b ) { return pygen( h ).rand( a, b ); } );
  m.def( "rand_gauss", []( Handle h, double sigma, double mu ) { return pygen( h ).randGauss( sigma, mu ); } );
  m.def( "rand_exponential", []( Handle h, double lambda ) { return pygen( h ).randExponential( lambda ); } );
  m.def( "rand_poisson", []( Handle h, double mu ) { return pygen( h ).randPoisson( mu ); } );
  m.def( "rand_int", []( Handle h, std::int64_t n ) { return pygen( h ).randInt( toLong( n ) ); } );
  m.def( "rand_int_range", []( Handle h, std::int64_t a, std::int64_t b ) { return pygen( h ).randInt( toLong( a ), toLong( b ) ); } );
  m.def( "rand_isotropic", []()
  {
    G4ThreeVector isodir = G4Utils::randIsotropicDirection();
    return Vect( isodir.x(), isodir.y(), isodir.z() );
  } );
  m.def( "particle_by_name", &particleByName );
  m.def( "particle_by_pdg", &particleByPDG );
}
