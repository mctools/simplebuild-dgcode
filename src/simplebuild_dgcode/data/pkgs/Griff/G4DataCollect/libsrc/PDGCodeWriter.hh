#ifndef G4DataCollect_PDGCodeWriter_hh
#define G4DataCollect_PDGCodeWriter_hh

#include "EvtFile/IDBSubSectionWriter.hh"
#include "EvtFile/Defs.hh"
#include "Utils/FastLookupSet.hh"
#include <set>
#include <vector>
#include "G4ParticleDefinition.hh"
#include <cassert>

namespace G4DataCollectInternals {

  struct DCMgr;

  class PDGCodeWriter : public EvtFile::IDBSubSectionWriter {
  public:
    PDGCodeWriter(EvtFile::subsectid_type sid,
                  EvtFile::FileWriter& fw,
                  DCMgr* mgr)
      : IDBSubSectionWriter(fw),
        m_mgr(mgr),
        m_ssid(sid),
        m_pdef_OpticalPhoton(0),
        m_pdef_Geantino(0)
    {
      m_pdgcodesToWrite.reserve(64);
    }
    virtual ~PDGCodeWriter(){}
    virtual EvtFile::subsectid_type uniqueSubSectionID() const { return m_ssid; }
    virtual bool needsWrite() const { return !m_pdgcodesToWrite.empty(); }
    virtual void write(EvtFile::FileWriter&);

    void registerPDGCode(int32_t pdg)
    {
      if (!m_pdgcodes.contains(pdg))
        {
          m_pdgcodesToWrite.push_back(pdg);
          m_pdgcodes.insert(pdg);
        }
    }


    //some particles (opticalphotons, geantinos ...) have 0 pdgcode. We create a new unique one with this method.
    int32_t getAdHocPDGCode(const G4ParticleDefinition*);
    const G4ParticleDefinition* getParticleDefinitionForAdHocPDGCode(int32_t pdgcode);
  private:
    Utils::FastLookupSet<int32_t> m_pdgcodes;
    std::vector<int32_t> m_pdgcodesToWrite;
    DCMgr* m_mgr;
    EvtFile::subsectid_type m_ssid;

    const G4ParticleDefinition * m_pdef_OpticalPhoton;
    const G4ParticleDefinition * m_pdef_Geantino;
  };


  inline int32_t PDGCodeWriter::getAdHocPDGCode(const G4ParticleDefinition*pd)
  {
    assert(pd);

    if (pd==m_pdef_OpticalPhoton) return -22;
    if (pd==m_pdef_Geantino) return 999;

    //[add other particles here as needed]

    //first time this kind of pdgcode==0 particle is seen, figure out what it is based on naming:
    if (pd->GetParticleName()=="opticalphoton") {
      m_pdef_OpticalPhoton = pd;
      return getAdHocPDGCode(pd);
    }

    if (pd->GetParticleName()=="geantino") {
      //NB: "chargedgeantino" is another particle, but we should probably map it
      //to a separate pdgcode (and use sign of the pdgcode for its sign).
      m_pdef_Geantino = pd;
      return getAdHocPDGCode(pd);
    }

    //[add other particles here as needed]

    printf("ERROR: New kind of particle with pdgcode==0 seen. Either this is a bug or PDGCodeWriter needs to be updated.\n");
    assert(false);
    return 0;//
  }

  inline const G4ParticleDefinition* PDGCodeWriter::getParticleDefinitionForAdHocPDGCode(int32_t pdgcode)
  {
    if (pdgcode==-22) return m_pdef_OpticalPhoton;
    if (pdgcode==999) return m_pdef_Geantino;
    //[add other particles here as needed]
    return 0;
  }

}

#endif
