#include "PDGCodeWriter.hh"
#include "EvtFile/FileWriter.hh"
#include "GriffFormat/ParticleDefinition.hh"
#include "G4Interfaces/FrameworkGlobals.hh"
#include "DCMgr.hh"
#include "G4ParticleTable.hh"

void G4DataCollectInternals::PDGCodeWriter::write(EvtFile::FileWriter&fw)
{
  fw.writeDataDBSection((std::uint8_t)0);//version
  fw.writeDataDBSection((std::uint32_t)m_pdgcodesToWrite.size());//number of pdg entries

  auto it = m_pdgcodesToWrite.begin();
  auto itE = m_pdgcodesToWrite.end();

  GriffFormat::ParticleDefinition pdef;

  for (;it!=itE;++it) {
    const G4ParticleDefinition * particle = G4ParticleTable::GetParticleTable()->FindParticle(*it);
    if (!particle&&*it) {
      //could be something with a raw g4 pdgcode of 0 like opticalphoton:
      particle = getParticleDefinitionForAdHocPDGCode(*it);
    }
    if (!particle) {
      printf("%sERROR: Did not find particle table for pdg code %i\n",FrameworkGlobals::printPrefix(),*it);
      assert(false);
      exit(1);
    }
    assert(particle);
    pdef.mass=particle->GetPDGMass();
    pdef.width=particle->GetPDGWidth();
    pdef.charge=particle->GetPDGCharge();
    pdef.lifeTime=particle->GetPDGLifeTime();
    pdef.nameIdx    = m_mgr->dbPDGNames.getIndex(particle->GetParticleName());
    pdef.typeIdx    = m_mgr->dbPDGTypes.getIndex(particle->GetParticleType());
    pdef.subTypeIdx = m_mgr->dbPDGSubTypes.getIndex(particle->GetParticleSubType());
    pdef.pdgcode = *it;
    pdef.atomicNumber = particle->GetAtomicNumber();
    pdef.atomicMass = particle->GetAtomicMass();
    pdef.magneticMoment = particle->GetPDGMagneticMoment();
    pdef.spin_halfs = (int16_t)(particle->GetPDGSpin()*2+0.5);
    pdef.stable = particle->GetPDGStable();
    pdef.shortLived = particle->IsShortLived();
    pdef.write(fw);
  }

  m_pdgcodesToWrite.clear();
}

