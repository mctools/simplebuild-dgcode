#include "G4StdGenerators/ProfiledBeamGen.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "Utils/NeutronMath.hh"
#include "Utils/MaxwellDist.hh"
#include "Units/Units.hh"
#include <cassert>
#include <stdexcept>

struct ProfiledBeamGen::Imp
{
  Imp(ProfiledBeamGen*gg) : m_g(gg), m_gun(0), m_parsinit(false) {}
  ~Imp() { delete m_gun; }

  void setup_gun()
  {
    assert(!m_gun);
    assert(m_parsinit);
    m_gun = new G4ParticleGun(1);
    G4ParticleDefinition* particle(0);
    if (par_particleName.empty()) {
      particle = G4ParticleTable::GetParticleTable()->FindParticle(par_pdgCode);
      if (!particle) {
        printf("ProfiledBeamGen ERROR: Could not find particle with pdg code %i\n",par_pdgCode);
        throw std::runtime_error("ProfiledBeamGen ERROR: Could not find particle");
      }
    } else {
      particle = G4ParticleTable::GetParticleTable()->FindParticle(par_particleName.c_str());
      if (!particle) {
        printf("ProfiledBeamGen ERROR: Could not find particle with name %s\n",par_particleName.c_str());
        throw std::runtime_error("ProfiledBeamGen ERROR: Could not find particle");
      }
    };
    assert(particle&&"unknown particle");
    m_gun->SetParticleDefinition(particle);

    m_gun->SetParticlePosition(G4ThreeVector(0,0,0));//to be overridden each event in case of beamspread
    m_gun->SetParticleMomentumDirection(G4ThreeVector(0,0,1));

    const bool is_neutron = (par_particleName.empty() ? par_pdgCode==2112 : par_particleName=="neutron");
    if (is_neutron&&par_neutron_wavelength) {
      if (!par_neutron_wavelength_spread) {
        //simply convert the wavelength to energy and setup pars for pure energy mode
        par_energy = Utils::neutronWavelengthToEKin(par_neutron_wavelength);
        par_neutron_wavelength = 0;
      }
    }
    if (par_energy)
      m_gun->SetParticleEnergy(par_energy);
  }

  void gen(G4Event*evt)
  {
    //The actual generation
    assert(m_gun&&m_parsinit);
    unsigned nparticles(0);
    while (!nparticles)//guard against null events
      nparticles = par_randomize_nparticles ? m_g->randPoisson(par_random_nparticles_poissonmean) : par_fixed_nparticles;

    assert(par_spread_mode>=0&&par_spread_mode<=2);

    for (unsigned iparticle=0;iparticle<nparticles;++iparticle) {
      double dx(0), dy(0);
      if (par_spread_mode==0) {
        //gaussian
        dx = par_spread_x ? m_g->randGauss(par_spread_x) : 0;
        dy = par_spread_y ? m_g->randGauss(par_spread_y) : 0;
      } else {
        while (true) {
          //flat - rectangle or circular/ellipsoidal:
          dx = par_spread_x ? m_g->rand(-par_spread_x,par_spread_x) : 0;
          dy = par_spread_y ? m_g->rand(-par_spread_y,par_spread_y) : 0;
          if (par_spread_mode==2)
            break;//rectangle
          if (dx*dx/(par_spread_x*par_spread_x)+dy*dy/(par_spread_y*par_spread_y) <= 1.0)
            break;//inside the ellipse
        }
      }

      m_gun->SetParticlePosition(G4ThreeVector( par_offset_x + dx,
                                                par_offset_y + dy,
                                                par_offset_z ));
      if (par_neutron_thermal_spectrum) {
        m_gun->SetParticleEnergy(Utils::shootThermalNeutronEKin(par_neutron_thermal_spectrum, m_g->rand()));
      } else if (par_neutron_wavelength_spread) {
        double wl = par_neutron_wavelength+m_g->randGauss(par_neutron_wavelength_spread);
        m_gun->SetParticleEnergy(wl > 0 ? Utils::neutronWavelengthToEKin(wl) : 10000.0*Units::GeV);
      }

      //Add the particle:
      m_gun->GeneratePrimaryVertex(evt);
    }
  }

  ProfiledBeamGen * m_g;
  G4ParticleGun * m_gun;
  bool m_parsinit;

  //Parameters, cached here for efficiency:
  double par_spread_x;
  double par_spread_y;
  int par_spread_mode;
  double par_offset_x;
  double par_offset_y;
  double par_offset_z;
  double par_energy;
  double par_neutron_wavelength;
  double par_neutron_wavelength_spread;
  double par_neutron_thermal_spectrum;
  int par_fixed_nparticles;
  bool par_randomize_nparticles;
  double par_random_nparticles_poissonmean;
  G4String par_particleName;
  int par_pdgCode;

  bool validatePars() {
    if (par_spread_mode<0) {
      printf("ProfiledBeamGen ERROR: spread_mode must be one of \"GAUSSIAN\", \"FLATCIRCULAR\" and \"FLATRECT\"\n");
      return false;
    }
    if (!par_particleName.empty()&&par_pdgCode) {
      printf("ProfiledBeamGen ERROR: Please do not set both particleName and pdgCode\n");
      return false;
    }
    if (par_particleName.empty()&&!par_pdgCode) {
      par_particleName="neutron";
      m_g->setParameterString("particleName",par_particleName);
    }
    double default_energy_eV = 100.0;
    const bool is_neutron = (par_particleName=="neutron"||par_pdgCode==2112);
    if (is_neutron) {
      if (par_neutron_wavelength_spread&&!par_neutron_wavelength) {
        printf("ProfiledBeamGen ERROR: Please do not set neutron_wavelength_spread_aangstrom without setting neutron_wavelength_aangstrom\n");
        return false;
      }
    } else {
      if (par_neutron_wavelength||par_neutron_wavelength_spread||par_neutron_thermal_spectrum) {
        printf("ProfiledBeamGen ERROR: Please do not set neutron_... parameters when not generating neutrons\n");
        return false;
      }
    }
    if (is_neutron) {
      unsigned n = (par_energy?1:0)+(par_neutron_wavelength?1:0)+(par_neutron_thermal_spectrum?1:0);
      if (n > 1 ) {
        printf("ProfiledBeamGen ERROR: Don't set more than one of neutron_wavelength_aangstrom, energy_eV and neutron_thermal_spectrum_kelvin\n");
        return false;
      }
      if (n==0) {
        par_energy = default_energy_eV*Units::eV;
        m_g->setParameterDouble("energy_eV",default_energy_eV);
      }
      //Ensure recorded consistency in energy/wavelength parameters:
      if (par_neutron_wavelength) {
        par_energy = Utils::neutronWavelengthToEKin(par_neutron_wavelength);
        m_g->setParameterDouble("energy_eV",par_energy/Units::eV);
      } else if (par_energy) {
        par_neutron_wavelength = Utils::neutronEKinToWavelength(par_energy);
        m_g->setParameterDouble("neutron_wavelength_aangstrom",par_neutron_wavelength/Units::angstrom);
      } else if (par_neutron_thermal_spectrum) {
        par_energy = 0.0;
        par_neutron_wavelength = 0.0;
      }
    } else {
      if (!par_energy) {
        par_energy = default_energy_eV*Units::eV;
        m_g->setParameterDouble("energy_eV",default_energy_eV);
      }
    }
    return true;
  }
};

ProfiledBeamGen::ProfiledBeamGen()
  : ParticleGenBase("G4StdGenerators/ProfiledBeamGen"),
    m_imp(0)
{
  ///////////////////////////////
  //  Beam spread and position //
  ///////////////////////////////

  addParameterDouble("spread_x_mm",0,0,1.0e5);
  addParameterDouble("spread_y_mm",0,0,1.0e5);

  addParameterString("spread_mode","GAUSSIAN");//"GAUSSIAN", "FLATCIRCULAR", "FLATRECT"

  addParameterDouble("offset_x_mm",0,-1.0e5,1.0e5);
  addParameterDouble("offset_y_mm",0,-1.0e5,1.0e5);
  addParameterDouble("offset_z_mm",0,-1.0e5,1.0e5);

  ////////////////////////////////////////////////////////////////
  //  Particle initial kinetic energy  (use exactly one method) //
  ////////////////////////////////////////////////////////////////

  //Particle kinetic energy
  addParameterDouble("energy_eV",0,0.0,10.0e12);

  //can only be set if particle is neutron:
  addParameterDouble("neutron_wavelength_aangstrom",0,0,1.0e3);
  addParameterDouble("neutron_wavelength_spread_aangstrom",0,0,1.0e3);
  addParameterDouble("neutron_thermal_spectrum_kelvin",0,0,10000);


  ////////////////////////////////////////////////
  //  Number of particles generated each event  //
  ////////////////////////////////////////////////

  addParameterInt("fixed_nparticles",1,1,100000);
  addParameterBoolean("randomize_nparticles",false);
  addParameterDouble("random_nparticles_poissonmean",1.0,1.0e-5,1.0e5);

  /////////////////////////
  //  Particle identity  //
  /////////////////////////

  //The user must call at most one of these methods:
  addParameterString("particleName","");
  addParameterInt("pdgCode",0);
}

ProfiledBeamGen::~ProfiledBeamGen()
{
  delete m_imp;
}

bool ProfiledBeamGen::validateParameters()
{
  if (!m_imp) m_imp = new Imp(this);

  m_imp->par_spread_x = getParameterDouble("spread_x_mm")*Units::mm;
  m_imp->par_spread_y = getParameterDouble("spread_y_mm")*Units::mm;
  m_imp->par_offset_x = getParameterDouble("offset_x_mm")*Units::mm;
  m_imp->par_offset_y = getParameterDouble("offset_y_mm")*Units::mm;
  m_imp->par_offset_z = getParameterDouble("offset_z_mm")*Units::mm;
  auto sm = getParameterString("spread_mode");
  if (sm=="GAUSSIAN") m_imp->par_spread_mode = 0;
  else if (sm=="FLATCIRCULAR") m_imp->par_spread_mode = 1;
  else if (sm=="FLATRECT") m_imp->par_spread_mode = 2;
  else m_imp->par_spread_mode = -1;//error
  //Make slightly more efficient for pencil-beams by always using the GAUSSIAN code-path:
  if (m_imp->par_spread_mode<=2&&m_imp->par_spread_x==0.0&&m_imp->par_spread_y==0.0)
    m_imp->par_spread_mode=0;
  m_imp->par_energy = getParameterDouble("energy_eV")*Units::eV;
  m_imp->par_neutron_wavelength = getParameterDouble("neutron_wavelength_aangstrom")*Units::angstrom;
  m_imp->par_neutron_wavelength_spread = getParameterDouble("neutron_wavelength_spread_aangstrom")*Units::angstrom;
  m_imp->par_neutron_thermal_spectrum = getParameterDouble("neutron_thermal_spectrum_kelvin")*Units::kelvin;
  m_imp->par_fixed_nparticles = getParameterInt("fixed_nparticles");
  m_imp->par_randomize_nparticles = getParameterBoolean("randomize_nparticles");
  m_imp->par_random_nparticles_poissonmean = getParameterDouble("random_nparticles_poissonmean");
  m_imp->par_particleName = getParameterString("particleName");
  m_imp->par_pdgCode = getParameterInt("pdgCode");
  m_imp->m_parsinit = true;

  return m_imp->validatePars();
}

void ProfiledBeamGen::init()
{
  if (!m_imp) {
    m_imp = new Imp(this);
  }
  if (!m_imp->m_parsinit)
    getParameterInt("pdgCode");//trigger parameter validation if not done already
  assert(m_imp&&m_imp->m_parsinit);
  m_imp->setup_gun();
}

void ProfiledBeamGen::gen(G4Event*evt)
{
  assert(m_imp);
  m_imp->gen(evt);
}
