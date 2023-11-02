#ifndef GriffDataRead_Element_hh
#define GriffDataRead_Element_hh

#include "EvtFile/DBEntryReader.hh"
#include "EvtFile/Defs.hh"
#include "Utils/ByteStream.hh"
#include "Core/Types.hh"
#include <vector>

class GriffDataReader;

namespace GriffDataRead {

  class Isotope;
  class Material;

  class Element {
  public:

    const std::string& getName() const;
    const char* getNameCStr() const;

    const std::string& getSymbol() const;
    const char* getSymbolCStr() const;

    double Z() const;
    double N() const;
    double A() const;

    bool naturalAbundances() const;

    //Isotope access:
    unsigned numberIsotopes() const;
    double isotopeRelativeAbundance(unsigned i_isotope) const;
    const Isotope* getIsotope(unsigned i_isotope) const;

  private:
    friend class EvtFile::DBEntryReader<Element>;

    Element(const char*&data);

    //data:
    GriffDataReader * m_dr;
    EvtFile::index_type m_nameIdx;
    EvtFile::index_type m_symbolIdx;
    double m_Z;
    double m_N;
    double m_A;
    bool m_naturalabundances;

    struct IsotopeInfo {
      double relativeabundance;
      EvtFile::index_type index;
      const Isotope * object;
    };
    mutable std::vector<IsotopeInfo> m_isotopes;
    friend class Material;
  };

}

#include "GriffDataRead/Element.icc"

#endif
