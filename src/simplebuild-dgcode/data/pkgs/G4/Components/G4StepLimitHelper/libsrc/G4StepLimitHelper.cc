#include "G4StepLimitHelper/G4StepLimitHelper.hh"
#include "G4LogicalVolume.hh"
#include "G4UserLimits.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4StepLimiter.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4RegionStore.hh"

#include "G4Region.hh"
#include "G4ProcessVector.hh"
#include "G4ProcessManager.hh"
G4StepLimitHelper::G4StepLimitHelper()  {
}

G4StepLimitHelper::~G4StepLimitHelper()  {
}

void G4StepLimitHelper::setWorldLimit()  {

  G4Region* deafultRegion = G4RegionStore::GetInstance()->GetRegion("DefaultRegionForTheWorld");
  if(!deafultRegion)
    G4Exception("G4StepLimitHelper::setWorldLimit:","",FatalException,"the world volume is not in its default name DefaultRegionForTheWorld.");

  printf( "G4StepLimitHelper::setWorldLimit Setting step limits for the world. If the volume limits conflict with the world limits, the volume limits win. \n") ;
  for(unsigned idx=0;idx<m_world_pgdCode.size();idx++)  {
    G4int pgdCode=m_world_pgdCode.at(idx);
    G4double uStepMax = m_world_uStepMax.at(idx);
    set(deafultRegion,pgdCode,uStepMax);
  }
}

void G4StepLimitHelper::addWorldLimit(G4int pgdCode, G4double uStepMax )  {
  m_world_pgdCode.push_back(pgdCode);
  m_world_uStepMax.push_back(uStepMax);
}


template <typename volume>
void G4StepLimitHelper::set(volume *v, G4int pgdCode, G4double uStepMax)
{
  const G4ParticleDefinition * particle=  G4ParticleTable::GetParticleTable()->FindParticle(pgdCode);
  printf( "G4StepLimitHelper::set Setting limit for particle PDG code %i", pgdCode );
  if(particle) {
    printf(", particle name \"%s\", step limit %.02emm, volume name \"%s\"\n", particle->GetParticleName().c_str(), uStepMax,  v->GetName().c_str());
    G4ProcessManager* pmanager = particle->GetProcessManager();
    if (pmanager)  {

      G4UserLimits *limt=new G4UserLimits(uStepMax);
      v->SetUserLimits(limt);
      bool model_exist=false;

      const G4ProcessVector* pl = pmanager->GetProcessList();
      auto size_pl = pl->size();
      for (decltype(size_pl) i=0;i<pl->size();++i) {
        if (!pmanager->GetProcessActivation(i))
          continue;
        G4StepLimiter* model = dynamic_cast<G4StepLimiter*>((*pl)[i]);
        if (model) {
          model_exist=true;
          break;
        }
      }
      if(!model_exist)   {
        G4StepLimiter *limiter = new G4StepLimiter();
        pmanager->AddDiscreteProcess(limiter);
      }
    }
    else
      G4Exception("setLimit by user:","",FatalException,"undefined particle manager");

  }
  else
    G4Exception("setLimit by user:","",FatalException,"invalid pdg code");
}


void G4StepLimitHelper::setLimit()  {

  for(unsigned idx=0;idx<m_pgdCode.size();idx++)  {

    G4String volName_str = m_volName.at(idx);
    G4int pgdCode=m_pgdCode.at(idx);
    G4double uStepMax = m_uStepMax.at(idx);

    G4LogicalVolumeStore* volStore = G4LogicalVolumeStore::GetInstance();
    G4LogicalVolume *vol = volStore->GetVolume(volName_str);
    if(!vol)
      G4Exception("setLimit by user:","",FatalException,"invalid logical volume name");

    set(vol,pgdCode,uStepMax);
  }
}


void G4StepLimitHelper::addLimit(G4int pgdCode,
    const char* volName,
    G4double uStepMax )
{
  m_pgdCode.push_back(pgdCode);
  m_volName.push_back(G4String(volName));
  m_uStepMax.push_back(uStepMax);
}




