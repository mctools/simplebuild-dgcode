#ifndef GriffDataRead_Touchable_hh
#define GriffDataRead_Touchable_hh

#include "EvtFile/DBEntryReader.hh"
#include "EvtFile/Defs.hh"
#include "Utils/ByteStream.hh"
#include "Core/Types.hh"

class GriffDataReader;

namespace GriffDataRead {

  class Material;

  class Touchable {
  public:

    unsigned storedDepth() const { return m_info.size(); }
    EvtFile::index_type volNameIdx(unsigned idepth=0) const;
    EvtFile::index_type physVolNameIdx(unsigned idepth=0) const;
    int copyNumber(unsigned idepth=0) const;
    const Material * material(unsigned idepth,GriffDataReader*dr) const;

  private:
    friend class EvtFile::DBEntryReader<Touchable>;
    Touchable(const char*&data);

    struct DepthInfo {
      EvtFile::index_type volNameIdx;
      EvtFile::index_type physVolNameIdx;
      EvtFile::index_type materialIdx;
      mutable const Material* material;
      int32_t copyNbr;
    };
    EvtFile::index_type m_volNameIdx0;//optimise often accessed variable
    std::vector<DepthInfo> m_info;
  };


  inline EvtFile::index_type GriffDataRead::Touchable::volNameIdx(unsigned idepth) const
  {
    assert(idepth<m_info.size());
    return idepth ? m_info[idepth].volNameIdx : m_volNameIdx0;
  }

  inline EvtFile::index_type GriffDataRead::Touchable::physVolNameIdx(unsigned idepth) const
  {
    assert(idepth<m_info.size());
    return m_info[idepth].physVolNameIdx;
  }

  inline int GriffDataRead::Touchable::copyNumber(unsigned idepth) const
  {
    assert(idepth<m_info.size());
    return m_info[idepth].copyNbr;
  }


}



#endif
