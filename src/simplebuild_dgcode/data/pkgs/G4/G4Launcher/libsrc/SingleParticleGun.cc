#include "G4Launcher/SingleParticleGun.hh"
#include "Units/Units.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include <cassert>

bool G4Launcher::SingleParticleGun::validateParameters()
{
  if (   !getParameterDouble("momdirx")
      && !getParameterDouble("momdiry")
      && !getParameterDouble("momdirz") )
    {
      printf("SingleParticleGun ERROR: Please supply momdir which is not null-vector\n");
      return false;
    }

  if (getParameterString("particleName").empty()) {
    if (getParameterInt("pdgCode")) {
      return true;
    } else {
      printf("SingleParticleGun ERROR: Please set either particleName or pdgCode\n");
      return false;
    }
  } else {
    if (getParameterInt("pdgCode")) {
      printf("SingleParticleGun ERROR: Please do not set both particleName and pdgCode\n");
      return false;
    } else {
      return true;
    }
  }
}

G4Launcher::SingleParticleGun::~SingleParticleGun()
{
  delete m_gun;
}

void G4Launcher::SingleParticleGun::init()
{
  assert(!m_gun);
  m_gun = new G4ParticleGun(getParameterInt("nParticles"));
  m_gun->SetParticleEnergy(getParameterDouble("energy_eV")*Units::eV);
  m_gun->SetParticlePosition(G4ThreeVector(getParameterDouble("x_meter")*Units::meter,
                                           getParameterDouble("y_meter")*Units::meter,
                                           getParameterDouble("z_meter")*Units::meter));
  m_gun->SetParticleMomentumDirection(G4ThreeVector(getParameterDouble("momdirx"),
                                                    getParameterDouble("momdiry"),
                                                    getParameterDouble("momdirz")));
  const std::string& pname = getParameterString("particleName");
  G4ParticleDefinition* particle(0);
  if (pname.empty()) {
    int pdgcode = getParameterInt("pdgCode");
    particle = G4ParticleTable::GetParticleTable()->FindParticle(pdgcode);
    if (!particle)
      printf("ERROR: Could not find particle with pdg code %i\n",pdgcode);
  } else {
    particle = G4ParticleTable::GetParticleTable()->FindParticle(pname.c_str());
    if (!particle)
      printf("ERROR: Could not find particle with name %s\n",pname.c_str());
  };
  assert(particle&&"unknown particle");
  m_gun->SetParticleDefinition(particle);
}

void G4Launcher::SingleParticleGun::gen(G4Event* evt)
{
  assert(m_gun);
  m_gun->GeneratePrimaryVertex(evt);
}
