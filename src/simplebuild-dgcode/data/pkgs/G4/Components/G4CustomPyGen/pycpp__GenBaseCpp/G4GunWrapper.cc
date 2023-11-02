#include "G4GunWrapper.hh"
#include "Units/Units.hh"
#include "Utils/NeutronMath.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"

G4CustomPyGen::G4GunWrapper::G4GunWrapper()
  : m_currentEvt(0),
    m_weight(1.0),
    m_nfired(0),
    m_empty_allowed(false),
    m_lastpdg(0)
{
  m_gun = new G4ParticleGun(1);
  set_type("neutron");
  set_energy(25*Units::meV);//fixme: set_wavelength_angstrom(1.8)
  set_position(0,0,0);
  set_direction(0,0,1);
}

G4CustomPyGen::G4GunWrapper::~G4GunWrapper()
{
  delete m_gun;
}

void G4CustomPyGen::G4GunWrapper::set_type(const char * particle_name)
{
  assert(particle_name);
  G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(particle_name);
  if (!particle) {
    printf("G4CustomPyGen set_type ERROR: Could not find particle with name %s\n",particle_name);
    throw std::runtime_error("G4CustomPyGen set_type ERROR: Could not find particle");
  }
  assert(particle&&"unknown particle");
  m_gun->SetParticleDefinition(particle);
  m_lastpdg = particle->GetPDGEncoding();
}

void G4CustomPyGen::G4GunWrapper::set_type(int pdg_code)
{
  if (m_lastpdg == pdg_code)
    return;
  G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(pdg_code);
  if ( !particle && (pdg_code/100000000 == 10)) {
    //Not in ParticleTable and pdgcode is of form 10xxxxxxxx, so look for ion:
    particle = G4IonTable::GetIonTable()->GetIon(pdg_code);
  }

  if (!particle) {
    printf("G4CustomPyGen set_type ERROR: Could not find particle with pdg code %i\n",pdg_code);
    throw std::runtime_error("G4CustomPyGen set_type ERROR: Could not find particle");
  }
  assert(particle&&"unknown particle");
  m_gun->SetParticleDefinition(particle);
  m_lastpdg = pdg_code;
}

void G4CustomPyGen::G4GunWrapper::set_wavelength(double wl)
{
  if (m_lastpdg!=2112)
    throw std::runtime_error("G4CustomPyGen set_wavelength ERROR: set_wavelength method only supports neutrons so far.");
  set_energy(Utils::neutronWavelengthToEKin(wl));
}
