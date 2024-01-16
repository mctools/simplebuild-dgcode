#include "G4MCPL/G4MCPLUserFlags.hh"
#include "G4Interfaces/ParticleGenPyExport.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Interfaces/FrameworkGlobals.hh"
#include "MCPLExprParser/MCPLASTBuilder.hh"
#include "MCPL/mcpl.h"
#include "Core/FindData.hh"
#include "Units/Units.hh"
#include "Core/String.hh"
#include "G4ParticleGun.hh"
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Transform3D.h"
#include <map>
#include <stdexcept>
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"

class MCPLGen : public G4Interfaces::ParticleGenBase {
public:
  MCPLGen();
  virtual ~MCPLGen();
  void init();
  void gen(G4Event*);

  virtual bool unlimited() const { return false; }//only has the events available in the input

protected:
  bool validateParameters();

private:
  void skip_forward(unsigned nskip);
  void delayed_init();
  void setPDG(int);
  mcpl_file_t m_mcplfile;
  const mcpl_particle_t * m_p;//particle to be used for next generation
  bool m_unfiltered;
  G4ParticleGun * m_gun;
  std::map<int,G4ParticleDefinition*> m_pdg2pdef;
  int m_nprocs;
  int m_lastpdgcode;
  bool m_has_polarisation;
  MCPLExprParser::MCPLASTBuilder m_builder;
  ExprParser::Evaluator<bool> m_eval_filter;
  //Nb: A Transform contains a translation, but apparently does not apply it on
  //Vector3D's! So we keep the translation separate specifically.
  HepGeom::Vector3D<double> m_translation;
  HepGeom::Transform3D m_rotation;
  bool m_allow_zero_weight;
};

MCPLGen::MCPLGen()
  : ParticleGenBase("G4MCPLPlugins/MCPLGen"),
    m_p(0),
    m_unfiltered(true),
    m_gun(0),
    m_nprocs(0),
    m_lastpdgcode(0),
    m_has_polarisation(false),
    m_allow_zero_weight(false)
{
  m_mcplfile.internal = 0;

  //either "filename" or "pkgname/filename"
  addParameterString("input_file","");

  //filter expression for MCPLExprParser (default to selecting everything in the file):
  addParameterString("input_filter","(true)");

  //Coordinate system transformation - first translate, then rotate about x, y and z in that order.
  addParameterDouble("dx_meter",0.0,-1.0e5,1.0e5);
  addParameterDouble("dy_meter",0.0,-1.0e5,1.0e5);
  addParameterDouble("dz_meter",0.0,-1.0e5,1.0e5);
  addParameterDouble("rotx_degree",0.0,-360.0,360.0);
  addParameterDouble("roty_degree",0.0,-360.0,360.0);
  addParameterDouble("rotz_degree",0.0,-360.0,360.0);

  //Skip this many events at the beginning of the file (events not passing the
  //input_filter are not included in the count):
  addParameterInt("skip_events",0,0,2147483647);

  //If not set, input events with weight=0 triggers an error.
  addParameterBoolean("allow_zero_weight",false);

}

MCPLGen::~MCPLGen()
{
  if (m_mcplfile.internal)
    mcpl_close_file(m_mcplfile);
  delete m_gun;
}

bool MCPLGen::validateParameters()
{
  std::string fn = getParameterString("input_file");
  if (fn.empty()) {
    printf("MCPLGen ERROR - Please supply input_file parameter.\n");
    return false;
  }
  if (!Core::ends_with(fn,".mcpl")&&!Core::ends_with(fn,".mcpl.gz")) {
    printf("MCPLGen ERROR - Input file does not have .mcpl or .mcpl.gz extension: %s\n",fn.c_str());
    return false;
  }
  //Don't check here that the file exists (since user might be attempting to
  //print the parameters before setting the file name)

  std::string filter = getParameterString("input_filter");
  try {
    m_eval_filter = m_builder.createEvaluator<bool>(filter);
  } catch (ExprParser::InputError& e) {
    printf("MCPLGen ERROR - %s in filter expression : %s\n",e.epType(),e.epWhat());
    return false;
  }

  if (m_eval_filter.isConstant()&&!m_eval_filter())
    printf("MCPLGen WARNING - filter expression always rejects all particles\n");

  m_unfiltered = ( m_eval_filter.isConstant() && m_eval_filter() );

  return true;
}

void MCPLGen::init()
{
  //In case of multiprocessing we haven't forked yet and each process needs it's
  //own filehandle. So don't create the reader yet.

  const double dx = getParameterDouble("dx_meter")*Units::meter;
  const double dy = getParameterDouble("dy_meter")*Units::meter;
  const double dz = getParameterDouble("dz_meter")*Units::meter;
  const double rotx = getParameterDouble("rotx_degree")*Units::deg;
  const double roty = getParameterDouble("roty_degree")*Units::deg;
  const double rotz = getParameterDouble("rotz_degree")*Units::deg;
  m_translation.set( dx,dy,dz);
  m_rotation = HepGeom::RotateZ3D(rotz)* HepGeom::RotateY3D(roty) * HepGeom::RotateX3D(rotx);

  m_allow_zero_weight = getParameterBoolean("allow_zero_weight");

  m_gun = new G4ParticleGun(1);
}

void MCPLGen::skip_forward(unsigned nskip) {
  //must ignore nskip (filtered) events, as they are destined for other processes.
  if (m_unfiltered) {
    mcpl_skipforward(m_mcplfile,nskip);//OK if failed, we will catch it in next mcpl_read.
    return;
  }
  while (nskip) {
    auto p = mcpl_read(m_mcplfile);
    if (!p)
      return;//Failed, but OK - we will catch it in next mcpl_read.
    m_builder.setCurrentParticle(p);
    if (m_eval_filter())
      --nskip;
  }
}

void MCPLGen::delayed_init()
{
  //opening files should happen after fork() when multi-processing.
  std::string fn = getParameterString("input_file");
  std::string fn_resolved = Core::findData(fn);
  if (fn_resolved.empty()) {
    printf("MCPLGen ERROR - File specified in input_file parameter not found : \"%s\"\n",fn.c_str());
    throw std::runtime_error("MCPLGen: File specified in input_file parameter not found");
  }

  m_mcplfile = mcpl_open_file(fn_resolved.c_str());

  m_has_polarisation = mcpl_hdr_has_polarisation(m_mcplfile);

  unsigned nskip = getParameterInt("skip_events");
  if (nskip)
    skip_forward(nskip);

  m_nprocs = FrameworkGlobals::nProcs();

  if (m_nprocs!=1&&FrameworkGlobals::mpID())
    skip_forward(FrameworkGlobals::mpID());

  m_p = mcpl_read(m_mcplfile);
  if (m_unfiltered)
    return;
  while(m_p) {
    m_builder.setCurrentParticle(m_p);
    if (m_eval_filter())
      return;
    m_p = mcpl_read(m_mcplfile);
  }
}

void MCPLGen::setPDG(int p)
{
  assert(m_lastpdgcode!=p);
  m_lastpdgcode=p;
  if (!p) {
    throw std::runtime_error("MCPLGen: File contains invalid pdgcode (0). An input_"
                             "filter \"pdgcode!=0\" could allow the other particles"
                             " in the file to be used, if desired.");
  }
  G4ParticleDefinition* particle;
  auto it = m_pdg2pdef.find(p);
  if (it!=m_pdg2pdef.end()) {
    particle = it->second;
  } else {
    particle = G4ParticleTable::GetParticleTable()->FindParticle(p);
    if ( !particle && (p/100000000 == 10)) {
      //Not in ParticleTable and pdgcode is of form 10xxxxxxxx, so look for ion:
      particle = G4IonTable::GetIonTable()->GetIon(p);
    }
    if (!particle) {
      printf("MCPLGen ERROR: Could not find particle with code %i\n",p);
      throw std::runtime_error("MCPLGen: Could not find particle. An"
                               " input_filter could possibly be used "
                               "to filter out these particles, if desired.");
    }
    m_pdg2pdef[p] = particle;
  }

  m_gun->SetParticleDefinition(particle);
}

void MCPLGen::gen(G4Event* evt)
{
  if (!m_nprocs) {
    //First time, open file and ready m_p as the particle to generate in the
    //current process:
    delayed_init();
  }
  assert(m_nprocs);

  if (!m_p) {
    //Oups, we don't have a particle to generate in this event! Should only
    //happen if number of (filtered) particles in the file is less than the
    //number of processes:
    signalEndOfEvents(true);
    return;
  }

  if (m_lastpdgcode != m_p->pdgcode)
    setPDG(m_p->pdgcode);

  //To avoid spurious warnings from G4ParticleGun, we should set
  //MomentumDirection and Energy, and avoid setting just Momentum directly.

  //load position and momentum:
  HepGeom::Vector3D<double> pos(m_p->position[0],m_p->position[1],m_p->position[2]);
  pos *= CLHEP::cm;
  HepGeom::Vector3D<double> dir(m_p->direction[0],m_p->direction[1],m_p->direction[2]);

  //set with rotation & translation as requested:
  m_gun->SetParticleMomentumDirection( m_rotation * dir );
  pos += m_translation;
  m_gun->SetParticlePosition( m_rotation * pos);

  //Kinetic energy (SetParticleEnergy is used to set kinetic energy, not total energy):
  assert(CLHEP::MeV==1.0);
  m_gun->SetParticleEnergy(m_p->ekin);//already in MeV and CLHEP::MeV=1

  //Time:
  m_gun->SetParticleTime(m_p->time*CLHEP::millisecond);

  //Polarisation:
  if (m_has_polarisation) {
    m_gun->SetParticlePolarization(G4ThreeVector(m_p->polarisation[0],
                                                 m_p->polarisation[1],
                                                 m_p->polarisation[2]));
  }

  //Generate particle and set weight:
  assert(evt->GetNumberOfPrimaryVertex()==0);
  const int ivertex = 0;
  m_gun->GeneratePrimaryVertex(evt);
  if (m_p->weight!=1.0) {
    if ( !m_p->weight && !m_allow_zero_weight ) {
      throw std::runtime_error("MCPLGen: File contains particles with weight=0 and allow_zero_weight was not enabled.");
    }
    evt->GetPrimaryVertex(ivertex)->SetWeight(m_p->weight);
  }

  //User flags require a bit special treatment:
  if (m_p->userflags) {
    G4PrimaryVertex * pv = evt->GetPrimaryVertex(ivertex);
    assert(pv->GetNumberOfParticle()==1);
    G4PrimaryParticle * pp = pv->GetPrimary(0);
    assert(pp);
    pp->SetUserInformation(new G4MCPLUserFlags(m_p->userflags));
  }

  //Finally, find the particle we will simulate in the next event. If not found,
  //we signal this already (thus avoiding an ungraceful abort in the middle of
  //the next event):
  if (m_nprocs>1) {
    //skip over events destined for other processes:
    skip_forward(m_nprocs-1);
  }

  m_p = mcpl_read(m_mcplfile);
  if (!m_unfiltered) {
    while(m_p) {
      m_builder.setCurrentParticle(m_p);
      if (m_eval_filter())
        break;
      m_p = mcpl_read(m_mcplfile);
    }
  }
  if (!m_p)
    signalEndOfEvents(false);//signal that this will be the last event

}


PYTHON_MODULE( mod )
{
  ParticleGenPyExport::exportGen<MCPLGen>(mod,"MCPLGen");
}
