#include "FlexGen.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "Units/Units.hh"
#include "Utils/NeutronMath.hh"
#include <cassert>
#include <stdexcept>

struct FlexGen::Imp
{
  Imp(FlexGen*gg) : m_g(gg), m_gun(0), m_parsinit(false) {}
  ~Imp() { delete m_gun; }


  void applySphericalCoords(double theta, double phi, G4ThreeVector& v)
  {
    //TODO: To G4 Utils?
    //polar angle theta: 0..pi
    //azimuthal angle phi: 0..2pi
    if (theta==0.0) {
      v.set(0,0,1);
      return;
    }
    const double sintheta = sin(theta);
    v.set(sintheta*cos(phi),sintheta*sin(phi),cos(theta));
  }

  void setup_gun()
  {
    assert(!m_gun);
    assert(m_parsinit);
    m_gun = new G4ParticleGun(1);
    G4ParticleDefinition* particle(0);
    if (par_particleName.empty()) {
      particle = G4ParticleTable::GetParticleTable()->FindParticle(par_pdgCode);
      if ( !particle && (par_pdgCode/100000000 == 10)) {
        //Not in ParticleTable and pdgcode is of form 10xxxxxxxx, so look for ion:
        particle = G4IonTable::GetIonTable()->GetIon(par_pdgCode);
      }
      if (!particle) {
        printf("FlexGen ERROR: Could not find particle with pdg code %i\n",par_pdgCode);
        throw std::runtime_error("FlexGen ERROR: Could not find particle");
      }
    } else {
      particle = G4ParticleTable::GetParticleTable()->FindParticle(par_particleName.c_str());
      if (!particle) {
        printf("FlexGen ERROR: Could not find particle with name %s\n",par_particleName.c_str());
        throw std::runtime_error("FlexGen ERROR: Could not find particle");
      }
    };
    assert(particle&&"unknown particle");
    m_gun->SetParticleDefinition(particle);
    if (!derivedpar_random_position)
      m_gun->SetParticlePosition(G4ThreeVector(par_fixed_x,par_fixed_y,par_fixed_z));
    if (!par_randomize_energy)
      m_gun->SetParticleEnergy(par_fixed_energy);
    if (!derivedpar_random_direction) {
      if (par_momdir_spherical) {
        G4ThreeVector v;
        applySphericalCoords(par_fixed_polarangle, par_fixed_azimuthalangle, v);
        m_gun->SetParticleMomentumDirection(v);
      } else {
        m_gun->SetParticleMomentumDirection(G4ThreeVector(par_fixed_xdir,par_fixed_ydir,par_fixed_zdir));
      }
    }
  }

  void gen(G4Event*evt)
  {
    //The actual generation
    assert(m_gun&&m_parsinit);
    unsigned nparticles(0);
    while (!nparticles)//guard against null events
      nparticles = par_randomize_nparticles ? m_g->randPoisson(par_random_nparticles_poissonmean) : par_fixed_nparticles;
    for (unsigned iparticle=0;iparticle<nparticles;++iparticle) {
      if (derivedpar_random_position)
        m_gun->SetParticlePosition(G4ThreeVector(par_randomize_x ? m_g->rand(par_random_min_x,par_random_max_x) : par_fixed_x,
                                                 par_randomize_y ? m_g->rand(par_random_min_y,par_random_max_y) : par_fixed_y,
                                                 par_randomize_z ? m_g->rand(par_random_min_z,par_random_max_z) : par_fixed_z));
      if (par_randomize_energy)
        m_gun->SetParticleEnergy(m_g->rand(par_random_min_energy,par_random_max_energy));

      if (derivedpar_random_direction) {
        if (par_momdir_spherical) {
          double polar = par_randomize_polarangle ? std::acos(m_g->rand(par_random_cos_min_polarangle,par_random_cos_max_polarangle)) : par_fixed_polarangle;
          double azimuthal = par_randomize_azimuthalangle ? m_g->rand(par_random_min_azimuthalangle,par_random_max_azimuthalangle) : par_fixed_azimuthalangle;
          G4ThreeVector v;
          applySphericalCoords(polar, azimuthal, v);
          m_gun->SetParticleMomentumDirection(v);
        } else {
          m_gun->SetParticleMomentumDirection(G4ThreeVector(par_randomize_xdir ? m_g->rand(par_random_min_xdir,par_random_max_xdir) : par_fixed_xdir,
                                                            par_randomize_ydir ? m_g->rand(par_random_min_ydir,par_random_max_ydir) : par_fixed_ydir,
                                                            par_randomize_zdir ? m_g->rand(par_random_min_zdir,par_random_max_zdir) : par_fixed_zdir));
        }
      }
      //Add the particle:
      m_gun->GeneratePrimaryVertex(evt);
    }


  }

  FlexGen * m_g;
  G4ParticleGun * m_gun;
  bool m_parsinit;

  //Parameters, cached here for efficiency:

  bool par_randomize_x;
  double par_fixed_x;
  double par_random_min_x;
  double par_random_max_x;

  bool par_randomize_y;
  double par_fixed_y;
  double par_random_min_y;
  double par_random_max_y;

  bool par_randomize_z;
  double par_fixed_z;
  double par_random_min_z;
  double par_random_max_z;

  bool par_randomize_energy;
  double par_fixed_energy;
  double par_random_min_energy;
  double par_random_max_energy;
  double par_neutron_wavelength;

  bool par_momdir_spherical;

  double par_fixed_polarangle;
  bool par_randomize_polarangle;
  double par_random_cos_min_polarangle;
  double par_random_cos_max_polarangle;

  double par_fixed_azimuthalangle;
  bool par_randomize_azimuthalangle;
  double par_random_min_azimuthalangle;
  double par_random_max_azimuthalangle;

  double par_fixed_xdir;
  bool par_randomize_xdir;
  double par_random_min_xdir;
  double par_random_max_xdir;

  double par_fixed_ydir;
  bool par_randomize_ydir;
  double par_random_min_ydir;
  double par_random_max_ydir;

  double par_fixed_zdir;
  bool par_randomize_zdir;
  double par_random_min_zdir;
  double par_random_max_zdir;

  int par_fixed_nparticles;
  bool par_randomize_nparticles;
  double par_random_nparticles_poissonmean;

  G4String par_particleName;
  int par_pdgCode;

  bool derivedpar_random_position;
  bool derivedpar_random_direction;

  bool validatePars() {
    if (!derivedpar_random_direction&&!par_momdir_spherical) {
      if (!par_fixed_xdir&&!par_fixed_ydir&&!par_fixed_zdir)
        {
          printf("FlexGen ERROR: Please supply direction which is not null-vector\n");
          return false;
        }
    }
    if (!par_particleName.empty()&&par_pdgCode) {
      printf("FlexGen ERROR: Please do not set both particleName and pdgCode\n");
      return false;
    }
    if (par_particleName.empty()&&!par_pdgCode) {
      par_particleName="neutron";
      m_g->setParameterString("particleName",par_particleName);
    }
    if (par_randomize_x&&par_random_min_x>=par_random_max_x) {
      printf("FlexGen ERROR: random_min_x_meters must be less than random_max_x_meters\n");
      return false;
    }
    if (par_randomize_y&&par_random_min_y>=par_random_max_y) {
      printf("FlexGen ERROR: random_min_y_meters must be less than random_max_y_meters\n");
      return false;
    }
    if (par_randomize_z&&par_random_min_z>=par_random_max_z) {
      printf("FlexGen ERROR: random_min_z_meters must be less than random_max_z_meters\n");
      return false;
    }
    if (par_randomize_energy&&par_random_min_energy>=par_random_max_energy) {
      printf("FlexGen ERROR: random_min_energy_eV must be less than random_max_energy_eV\n");
      return false;
    }
    const bool is_neutron = (par_particleName.empty() ? par_pdgCode==2112 : par_particleName=="neutron");
    if (is_neutron) {
      if (par_neutron_wavelength&&par_fixed_energy) {
        printf("FlexGen ERROR: Don't set both neutron_wavelength_aangstrom and fixed_energy_eV\n");
        return false;
      }
      if (par_neutron_wavelength) {
        par_fixed_energy = Utils::neutronWavelengthToEKin(par_neutron_wavelength);
        m_g->setParameterDouble("fixed_energy_eV",par_fixed_energy/Units::eV);
      }
    } else {
      if (par_neutron_wavelength) {
        printf("FlexGen ERROR: Please do not set neutron_wavelength_aangstrom when not generating neutrons\n");
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

    if (par_momdir_spherical) {
      if (par_randomize_polarangle&&par_random_cos_min_polarangle>=par_random_cos_max_polarangle) {
        printf("FlexGen ERROR: random_min_polarangle_deg must be less than random_max_polarangle_deg\n");
        return false;
      }
      if (par_randomize_azimuthalangle&&par_random_min_azimuthalangle>=par_random_max_azimuthalangle) {
        printf("FlexGen ERROR: random_min_azimuthalangle_deg must be less than random_max_azimuthalangle_deg\n");
        return false;
      }
    } else {
      if (par_randomize_xdir&&par_random_min_xdir>=par_random_max_xdir) {
        printf("FlexGen ERROR: random_min_xdir must be less than random_max_xdir\n");
        return false;
      }
      if (par_randomize_ydir&&par_random_min_ydir>=par_random_max_ydir) {
        printf("FlexGen ERROR: random_min_ydir must be less than random_max_ydir\n");
        return false;
      }
      if (par_randomize_zdir&&par_random_min_zdir>=par_random_max_zdir) {
        printf("FlexGen ERROR: random_min_zdir must be less than random_max_zdir\n");
        return false;
      }
    }
    return true;
  }
};

FlexGen::FlexGen()
  : ParticleGenBase("G4StdGenerators/FlexGen"),
    m_imp(0)
{
  /////////////////////////////////
  //  Particle initial position  //
  /////////////////////////////////

  //Particle initial position - X-coordinate
  addParameterDouble("fixed_x_meters",0,-1.0e5,1.0e5);
  addParameterBoolean("randomize_x",false);
  addParameterDouble("random_min_x_meters",0,-1.0e5,1.0e5);
  addParameterDouble("random_max_x_meters",0,-1.0e5,1.0e5);

  //Particle initial position - Y-coordinate
  addParameterDouble("fixed_y_meters",0,-1.0e5,1.0e5);
  addParameterBoolean("randomize_y",false);
  addParameterDouble("random_min_y_meters",0,-1.0e5,1.0e5);
  addParameterDouble("random_max_y_meters",0,-1.0e5,1.0e5);

  //Particle initial position - Z-coordinate
  addParameterDouble("fixed_z_meters",0,-1.0e5,1.0e5);
  addParameterBoolean("randomize_z",false);
  addParameterDouble("random_min_z_meters",0,-1.0e5,1.0e5);
  addParameterDouble("random_max_z_meters",0,-1.0e5,1.0e5);

  ///////////////////////////////////////
  //  Particle initial kinetic energy  //
  ///////////////////////////////////////

  //Particle kinetic energy
  addParameterDouble("fixed_energy_eV",0,0.0,10.0e12);
  addParameterBoolean("randomize_energy",false);
  addParameterDouble("random_min_energy_eV",0,0.0,1.0e11);
  addParameterDouble("random_max_energy_eV",0,0.0,1.0e11);

  //Alternatively (for neutrons only) generate with a fixed wavelength
  addParameterDouble("neutron_wavelength_aangstrom",0,0.0,1.0e11);
  //NB: Particle must be a neutron and other energy pars will be ignored if set.

  ///////////////////////////////////////////
  //  Particle initial momentum direction  //
  ///////////////////////////////////////////

  //For initial momentum direction there is a choice between specifying it
  //in either cartesian or spherical coordinates
  addParameterBoolean("momdir_spherical",false);//used to be true, but must match SimpleGen

  //Spherical coordinates: polar angle (around Z-axis)
  addParameterDouble("fixed_polarangle_deg",0.,0.,180.);//if 0, then phi doesn't matter
  addParameterBoolean("randomize_polarangle",false);
  addParameterDouble("random_min_polarangle_deg",0.,0.,180.);
  addParameterDouble("random_max_polarangle_deg",180.,0.,180.);

  //Spherical coordinates: azimuthal angle (around Z-axis)
  addParameterDouble("fixed_azimuthalangle_deg",0.,0.,360.);
  addParameterBoolean("randomize_azimuthalangle",false);
  addParameterDouble("random_min_azimuthalangle_deg",0.,0.,360.);
  addParameterDouble("random_max_azimuthalangle_deg",360.,0.,360.);

  //Cartesian coordinates: x component of direction vector
  addParameterDouble("fixed_xdir",0,-1.0e5,1.0e5);
  addParameterBoolean("randomize_xdir",false);
  addParameterDouble("random_min_xdir",0,-1.0e5,1.0e5);
  addParameterDouble("random_max_xdir",0,-1.0e5,1.0e5);

  //Cartesian coordinates: y component of direction vector
  addParameterDouble("fixed_ydir",0,-1.0e5,1.0e5);
  addParameterBoolean("randomize_ydir",false);
  addParameterDouble("random_min_ydir",0,-1.0e5,1.0e5);
  addParameterDouble("random_max_ydir",0,-1.0e5,1.0e5);

  //Cartesian coordinates: z component of direction vector
  addParameterDouble("fixed_zdir",1,-1.0e5,1.0e5);
  addParameterBoolean("randomize_zdir",false);
  addParameterDouble("random_min_zdir",0,-1.0e5,1.0e5);
  addParameterDouble("random_max_zdir",0,-1.0e5,1.0e5);

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

FlexGen::~FlexGen()
{
  delete m_imp;
}

bool FlexGen::validateParameters()
{
  if (!m_imp) m_imp = new Imp(this);

  m_imp->par_randomize_x = getParameterBoolean("randomize_x");
  m_imp->par_fixed_x = getParameterDouble("fixed_x_meters")*Units::meter;
  m_imp->par_random_min_x = getParameterDouble("random_min_x_meters")*Units::meter;
  m_imp->par_random_max_x = getParameterDouble("random_max_x_meters")*Units::meter;

  m_imp->par_randomize_y = getParameterBoolean("randomize_y");
  m_imp->par_fixed_y = getParameterDouble("fixed_y_meters")*Units::meter;
  m_imp->par_random_min_y = getParameterDouble("random_min_y_meters")*Units::meter;
  m_imp->par_random_max_y = getParameterDouble("random_max_y_meters")*Units::meter;

  m_imp->par_randomize_z = getParameterBoolean("randomize_z");
  m_imp->par_fixed_z = getParameterDouble("fixed_z_meters")*Units::meter;
  m_imp->par_random_min_z = getParameterDouble("random_min_z_meters")*Units::meter;
  m_imp->par_random_max_z = getParameterDouble("random_max_z_meters")*Units::meter;

  m_imp->par_randomize_energy = getParameterBoolean("randomize_energy");
  m_imp->par_fixed_energy = getParameterDouble("fixed_energy_eV")*Units::eV;
  m_imp->par_random_min_energy = getParameterDouble("random_min_energy_eV")*Units::eV;
  m_imp->par_random_max_energy = getParameterDouble("random_max_energy_eV")*Units::eV;
  m_imp->par_neutron_wavelength = getParameterDouble("neutron_wavelength_aangstrom")*Units::angstrom;

  m_imp->par_momdir_spherical = getParameterBoolean("momdir_spherical");

  m_imp->par_fixed_polarangle = getParameterDouble("fixed_polarangle_deg")*M_PI/180.;
  m_imp->par_randomize_polarangle = getParameterBoolean("randomize_polarangle");
  m_imp->par_random_cos_max_polarangle = std::cos(getParameterDouble("random_min_polarangle_deg")*M_PI/180.);//cosine on 0..pi decreases
  m_imp->par_random_cos_min_polarangle = std::cos(getParameterDouble("random_max_polarangle_deg")*M_PI/180.);//and thus swaps min/max

  m_imp->par_fixed_azimuthalangle = getParameterDouble("fixed_azimuthalangle_deg")*M_PI/180.;
  m_imp->par_randomize_azimuthalangle = getParameterBoolean("randomize_azimuthalangle");
  m_imp->par_random_min_azimuthalangle = getParameterDouble("random_min_azimuthalangle_deg")*M_PI/180.;
  m_imp->par_random_max_azimuthalangle = getParameterDouble("random_max_azimuthalangle_deg")*M_PI/180.;

  m_imp->par_fixed_xdir = getParameterDouble("fixed_xdir");
  m_imp->par_randomize_xdir = getParameterBoolean("randomize_xdir");
  m_imp->par_random_min_xdir = getParameterDouble("random_min_xdir");
  m_imp->par_random_max_xdir = getParameterDouble("random_max_xdir");

  m_imp->par_fixed_ydir = getParameterDouble("fixed_ydir");
  m_imp->par_randomize_ydir = getParameterBoolean("randomize_ydir");
  m_imp->par_random_min_ydir = getParameterDouble("random_min_ydir");
  m_imp->par_random_max_ydir = getParameterDouble("random_max_ydir");

  m_imp->par_fixed_zdir = getParameterDouble("fixed_zdir");
  m_imp->par_randomize_zdir = getParameterBoolean("randomize_zdir");
  m_imp->par_random_min_zdir = getParameterDouble("random_min_zdir");
  m_imp->par_random_max_zdir = getParameterDouble("random_max_zdir");

  m_imp->par_fixed_nparticles = getParameterInt("fixed_nparticles");
  m_imp->par_randomize_nparticles = getParameterBoolean("randomize_nparticles");
  m_imp->par_random_nparticles_poissonmean = getParameterDouble("random_nparticles_poissonmean");

  m_imp->par_particleName = getParameterString("particleName");
  m_imp->par_pdgCode = getParameterInt("pdgCode");

  m_imp->derivedpar_random_position = m_imp->par_randomize_x || m_imp->par_randomize_y || m_imp->par_randomize_z;

  if (m_imp->par_momdir_spherical)
    m_imp->derivedpar_random_direction = m_imp->par_randomize_polarangle || m_imp->par_randomize_azimuthalangle;
  else
    m_imp->derivedpar_random_direction = m_imp->par_randomize_xdir || m_imp->par_randomize_ydir || m_imp->par_randomize_zdir;

  m_imp->m_parsinit = true;

  return m_imp->validatePars();
}

void FlexGen::init()
{
  if (!m_imp) {
    m_imp = new Imp(this);
  }
  if (!m_imp->m_parsinit)
    getParameterBoolean("randomize_x");//trigger parameter validation if not done already
  assert(m_imp&&m_imp->m_parsinit);
  m_imp->setup_gun();
}

void FlexGen::gen(G4Event*evt)
{
  assert(m_imp);
  m_imp->gen(evt);
}

