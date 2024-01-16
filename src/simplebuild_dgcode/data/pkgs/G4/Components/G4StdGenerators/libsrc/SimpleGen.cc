#include "G4StdGenerators/SimpleGen.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "Units/Units.hh"
#include "Utils/NeutronMath.hh"
#include <cassert>
#include <stdexcept>

struct SimpleGen::Imp
{
  Imp(SimpleGen*gg) : m_g(gg), m_gun(0), m_parsinit(false) {}
  ~Imp() { delete m_gun; }

  void setup_gun()
  {
    assert(!m_gun);
    assert(m_parsinit);
    m_gun = new G4ParticleGun(1);
    G4ParticleDefinition* particle(0);
    particle = G4ParticleTable::GetParticleTable()->FindParticle(par_particleName.c_str());
    if (!particle) {
      printf("SimpleGen ERROR: Could not find particle with name %s\n",par_particleName.c_str());
      throw std::runtime_error("SimpleGen ERROR: Could not find particle");
    }
    assert(particle&&"unknown particle");
    m_gun->SetParticleDefinition(particle);
    m_gun->SetParticlePosition(G4ThreeVector(par_fixed_x,par_fixed_y,par_fixed_z));
    m_gun->SetParticleEnergy(par_fixed_energy);
    m_gun->SetParticleMomentumDirection(G4ThreeVector(par_fixed_xdir,par_fixed_ydir,par_fixed_zdir));
  }

  void gen(G4Event*evt)
  {
    //The actual generation
    assert(m_gun&&m_parsinit);
    m_gun->GeneratePrimaryVertex(evt);
  }

  SimpleGen * m_g;
  G4ParticleGun * m_gun;
  bool m_parsinit;

  //Parameters:
  double par_fixed_x;
  double par_fixed_y;
  double par_fixed_z;
  double par_fixed_energy;
  double par_neutron_wavelength;
  double par_fixed_xdir;
  double par_fixed_ydir;
  double par_fixed_zdir;

  G4String par_particleName;


  bool validatePars() {
    if (!par_fixed_xdir&&!par_fixed_ydir&&!par_fixed_zdir)
      {
        printf("SimpleGen ERROR: Please supply direction which is not null-vector\n");
        return false;
      }
    if (par_particleName.empty()) {
      par_particleName="neutron";
      m_g->setParameterString("particleName",par_particleName);
    }
    const bool is_neutron = (par_particleName=="neutron");
    if (is_neutron) {
      if (par_neutron_wavelength&&par_fixed_energy) {
        printf("SimpleGen ERROR: Don't set both neutron_wavelength_aangstrom and fixed_energy_eV\n");
        return false;
      }
      if (par_neutron_wavelength) {
        par_fixed_energy = Utils::neutronWavelengthToEKin(par_neutron_wavelength);
        m_g->setParameterDouble("fixed_energy_eV",par_fixed_energy/Units::eV);
      }
    } else {
      if (par_neutron_wavelength) {
        printf("SimpleGen ERROR: Please do not set neutron_wavelength_aangstrom when not generating neutrons\n");
        return false;
      }
    }
    double default_energy_eV = 100.0;
    if (!par_fixed_energy) {
      par_fixed_energy = default_energy_eV*Units::eV;
      m_g->setParameterDouble("fixed_energy_eV",default_energy_eV);
    }
    if (is_neutron&&!par_neutron_wavelength) {
      par_neutron_wavelength = Utils::neutronEKinToWavelength(par_fixed_energy);
      m_g->setParameterDouble("neutron_wavelength_aangstrom",par_neutron_wavelength/Units::angstrom);
    }
    return true;
  }
};

SimpleGen::SimpleGen()
  : ParticleGenBase("G4StdGenerators/SimpleGen"),
    m_imp(0)
{
  /////////////////////////////////
  //  Particle initial position  //
  /////////////////////////////////

  //Particle initial position - XYZ-coordinate
  addParameterDouble("fixed_x_meters",0,-1.0e5,1.0e5);
  addParameterDouble("fixed_y_meters",0,-1.0e5,1.0e5);
  addParameterDouble("fixed_z_meters",0,-1.0e5,1.0e5);

  ///////////////////////////////////////
  //  Particle initial kinetic energy  //
  ///////////////////////////////////////

  //Particle kinetic energy
  addParameterDouble("fixed_energy_eV",0,0.0,10.0e12);

  //Alternatively (for neutrons only) generate with a fixed wavelength
  addParameterDouble("neutron_wavelength_aangstrom",0,0.0,1.0e11);
  //NB: Particle must be a neutron and fixed_energy_eV should not be set.

  ///////////////////////////////////////////
  //  Particle initial momentum direction  //
  ///////////////////////////////////////////

  //Cartesian coordinates: xyz component of direction vector
  addParameterDouble("fixed_xdir",0,-1.0e5,1.0e5);
  addParameterDouble("fixed_ydir",0,-1.0e5,1.0e5);
  addParameterDouble("fixed_zdir",1,-1.0e5,1.0e5);

  /////////////////////////
  //  Particle identity  //
  /////////////////////////

  addParameterString("particleName","");
}

SimpleGen::~SimpleGen()
{
  delete m_imp;
}

bool SimpleGen::validateParameters()
{
  if (!m_imp) m_imp = new Imp(this);

  m_imp->par_fixed_x = getParameterDouble("fixed_x_meters")*Units::meter;
  m_imp->par_fixed_y = getParameterDouble("fixed_y_meters")*Units::meter;
  m_imp->par_fixed_z = getParameterDouble("fixed_z_meters")*Units::meter;
  m_imp->par_fixed_energy = getParameterDouble("fixed_energy_eV")*Units::eV;
  m_imp->par_neutron_wavelength = getParameterDouble("neutron_wavelength_aangstrom")*Units::angstrom;
  m_imp->par_fixed_xdir = getParameterDouble("fixed_xdir");
  m_imp->par_fixed_ydir = getParameterDouble("fixed_ydir");
  m_imp->par_fixed_zdir = getParameterDouble("fixed_zdir");
  m_imp->par_particleName = getParameterString("particleName");

  m_imp->m_parsinit = true;

  return m_imp->validatePars();
}

void SimpleGen::init()
{
  if (!m_imp) {
    m_imp = new Imp(this);
  }
  if (!m_imp->m_parsinit)
    getParameterBoolean("fixed_x_meters");//trigger parameter validation if not done already
  assert(m_imp&&m_imp->m_parsinit);
  m_imp->setup_gun();
}

void SimpleGen::gen(G4Event*evt)
{
  assert(m_imp);
  m_imp->gen(evt);
}

