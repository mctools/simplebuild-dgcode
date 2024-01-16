#ifndef GriffDataRead_Isotope_hh
#define GriffDataRead_Isotope_hh

#include "EvtFile/DBEntryReader.hh"
#include "EvtFile/Defs.hh"
#include "Utils/ByteStream.hh"
#include "Core/Types.hh"
#include <vector>

class GriffDataReader;

namespace GriffDataRead {

  class Element;

  class Isotope {
  public:

    const std::string& getName() const;
    const char* getNameCStr() const;

    int32_t Z() const;
    int32_t N() const;
    double A() const;
    int32_t m() const;//isomer level

  private:
    friend class EvtFile::DBEntryReader<Isotope>;

    Isotope(const char*&data);

    //data:
    GriffDataReader * m_dr;
    EvtFile::index_type m_nameIdx;
    int32_t m_Z;
    int32_t m_N;
    double m_A;
    int32_t m_m;
    friend class Element;
  };

}

#include "GriffDataRead/Isotope.icc"

#endif
