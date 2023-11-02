#include "Core/Types.hh"//must include first to build on all platforms
#include "GriffGen.hh"

#include "G4Interfaces/FrameworkGlobals.hh"
#include "GriffDataRead/GriffDataReader.hh"
#include "Core/FindData.hh"
#include "Core/String.hh"

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4Event.hh"

#include <stdexcept>

GriffGen::GriffGen()
  : ParticleGenBase("G4GriffGen/GriffGen"),
    m_dr(0),
    m_nprocs(1),
    m_primary_only(true)
{
  addParameterString("input_file","");
  addParameterBoolean("primary_only",true);
  addParameterInt("skip_events",0,0,2e9);//Skip this many events at the beginning of the file
  addParameterBoolean("allow_setup_change",false);
}

GriffGen::~GriffGen()
{
  delete m_dr;
}

bool GriffGen::validateParameters()
{
  std::string fn = getParameterString("input_file");
  if (fn.empty()) {
    printf("GriffGen ERROR - Please supply input_file parameter.\n");
    return false;
  }
  if (!Core::ends_with(fn,".griff")) {
    printf("GriffGen ERROR - Input file does not have .griff extension: %s\n",fn.c_str());
    return false;
  }
  if (Core::findData(fn).empty()) {
    printf("GriffGen ERROR - Input file not found: %s\n",fn.c_str());
    return false;
  }
  return true;
}

void GriffGen::init()
{
  m_gun = new G4ParticleGun(1);
  m_primary_only = getParameterBoolean("primary_only");
  //NB: Delay m_dr and m_nprocs initialisation to ::gen() to support multiprocessing.
}

void GriffGen::initDR()
{
  assert(!m_dr);
  m_dr = new GriffDataReader(Core::findData(getParameterString("input_file")));

  if (getParameterBoolean("allow_setup_change"))
    m_dr->allowSetupChange();

  unsigned nskip = getParameterInt("skip_events");
  if (nskip&&!m_dr->skipEvents(nskip)) {
    printf("GriffGen ERROR - Could not skip %i events (too few events in file?\n",nskip);
    throw std::runtime_error("GriffGen ERROR - skip event failure");
  }
}

void GriffGen::gen(G4Event*evt)
{
  //Init, loop events, etc. as needed to handle multiprocessing.
  int mustloop;
  if (!m_dr) {
    //first event
    m_nprocs = FrameworkGlobals::nProcs();
    initDR();//delayed init, so file handle gets created after fork.
    mustloop = m_nprocs > 1 ? FrameworkGlobals::mpID()+1 : 1;//proceed mpID extra events.
  } else {
    mustloop = m_nprocs;//proceed nprocs events
  }
  while (mustloop) {
    if (!m_dr->loopEvents()) {
      signalEndOfEvents(true);//todo: spy ahead and call signalEndOfEvents(false) in previous event instead
      return;
    }
    --mustloop;
  }

  //Decode and generate G4 event based on Griff event:

  assert(m_dr->eventActive());

  //Check that we have step info:
  if (m_dr->eventStorageMode()==GriffFormat::Format::MODE_MINIMAL)
    throw std::runtime_error("GriffGen ERROR - Can't generate particles based on events in Griff MINIMAL mode");

  //Simply reshoot based on first step on tracks in griff files. Either for all primary or all tracks:
  if (m_primary_only) {
    auto trkE = m_dr->primaryTrackEnd();
    for (auto trk = m_dr->primaryTrackBegin();trk!=trkE;++trk)
      shootPreStep(evt,trk->firstStep());
  } else {
    auto trkE = m_dr->trackEnd();
    for (auto trk = m_dr->trackBegin();trk!=trkE;++trk)
      shootPreStep(evt,trk->firstStep());
  }
}

void GriffGen::shootPreStep(G4Event*evt, const GriffDataRead::Step* step)
{
  int pdgcode = step->getTrack()->pdgCode();
  G4ParticleDefinition* particle(0);
  auto it = m_partdefs.find(pdgcode);
  if (it==m_partdefs.end()) {
    particle = G4ParticleTable::GetParticleTable()->FindParticle(pdgcode);
    if (!particle&&pdgcode==999) particle = G4ParticleTable::GetParticleTable()->FindParticle("geantino");
    if (!particle&&pdgcode==-22) particle = G4ParticleTable::GetParticleTable()->FindParticle("opticalphoton");
    if ( !particle && (pdgcode/100000000 == 10)) {
      //Not in ParticleTable and pdgcode is of form 10xxxxxxxx, so look for ion:
      particle = G4IonTable::GetIonTable()->GetIon(pdgcode);
    }
    if (!particle) {
      printf("GriffGen ERROR - Could not locate particle with pdgcode %i in present physics list\n",pdgcode);
      throw std::runtime_error("GriffGen ERROR - Unknown pdgcode");
    }
    m_partdefs[pdgcode] = particle;
  } else {
    particle = it->second;
  }
  m_gun->SetParticleDefinition(particle);
  m_gun->SetParticleTime(step->preTime());
  m_gun->SetParticlePosition(G4ThreeVector(step->preGlobalX(),step->preGlobalY(),step->preGlobalZ()));
  double ekin = step->preEKin();
  m_gun->SetParticleEnergy(ekin);
  if (ekin)
    m_gun->SetParticleMomentumDirection(G4ThreeVector(step->preMomentumX(),step->preMomentumY(),step->preMomentumZ()));
  else
    m_gun->SetParticleMomentumDirection(G4ThreeVector(0.0,0.0,1.0));
  int iv = evt->GetNumberOfPrimaryVertex();
  m_gun->GeneratePrimaryVertex(evt);
  double weight = step->getTrack()->weight();
  if (weight!=1.0)
    evt->GetPrimaryVertex(iv)->SetWeight(weight);
}
