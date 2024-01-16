#ifndef GriffDataRead_PDGCodeReader_hh
#define GriffDataRead_PDGCodeReader_hh

#include "Core/Types.hh"
#include "GriffFormat/ParticleDefinition.hh"
#include "EvtFile/IDBSubSectionReader.hh"
#include <map>

namespace GriffDataRead {

  class PDGCodeReader : public EvtFile::IDBSubSectionReader {
  public:
    PDGCodeReader(EvtFile::subsectid_type sid) : m_sid(sid) {}
    virtual ~PDGCodeReader(){}

    bool hasParticleDefinition(int pdgcode) const
    {
      return m_pdg.find(pdgcode)!=m_pdg.end();
    }

    const GriffFormat::ParticleDefinition* getParticleDefinition(int pdgcode) const
    {
      auto it = m_pdg.find(pdgcode);
      assert(it!=m_pdg.end());
      return &(it->second);
    }

    //Must be unique within a file format:
    virtual EvtFile::subsectid_type uniqueSubSectionID() const { return m_sid; }
    virtual void load(const char*&data)
    {
      std::uint8_t version;
      ByteStream::read(data,version);
      assert(version==0);//only known version so far.
      std::uint32_t n;//number of new pdg codes
      ByteStream::read(data,n);
      GriffFormat::ParticleDefinition pdef;
      for (unsigned i=0;i<n;++i) {
        ByteStream::read(data,pdef);//the ParticleDefinition object is carefully constructed to be devoid of padding.
        assert(sizeof(pdef)==64);//if not, something must have changed somewhere and the impact must be understood.
        assert(m_pdg.find(pdef.pdgcode)==m_pdg.end());
        m_pdg[pdef.pdgcode]=pdef;
        static_assert(sizeof(int)>=sizeof(pdef.pdgcode));
      }
    }
    virtual void clearInfo() { m_pdg.clear(); }
  private:
    EvtFile::subsectid_type m_sid;
    std::map<int,GriffFormat::ParticleDefinition> m_pdg;
  };

}
#endif
