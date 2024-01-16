#ifndef GriffDataRead_Material_hh
#define GriffDataRead_Material_hh

#include "EvtFile/DBEntryReader.hh"
#include "EvtFile/Defs.hh"
#include "Utils/ByteStream.hh"
#include "Core/Types.hh"
#include <vector>

class GriffDataReader;

namespace GriffDataRead {

  class Element;
  class Segment;
  class Material {
  public:

    const std::string& getName() const;
    const char* getNameCStr() const;

    double density() const;
    double temperature() const;
    double pressure() const;
    double radiationLength() const;
    double nuclearInteractionLength() const;

    bool hasMeanExitationEnergy() const;
    double meanExitationEnergy() const;

    //Exactly one of the next four methods will be true:
    bool isSolid() const;
    bool isGas() const;
    bool isLiquid() const;
    bool isUndefinedState() const;
    //Returns "Solid", "Gas", "Liquid" or "Undefined"
    const std::string& stateStr() const;
    const char* stateCStr() const;

    //Element access:
    unsigned numberElements() const;
    double elementFraction(unsigned ielem) const;
    const Element* getElement(unsigned ielem) const;

  private:
    friend class EvtFile::DBEntryReader<Material>;
    friend class Touchable;

    Material(const char*&data);

    //data:
    GriffDataReader * m_dr;
    EvtFile::index_type m_nameIdx;
    double m_density;
    double m_temperature;
    double m_pressure;
    double m_radlen;
    double m_nuclearinterlength;
    double m_ion_meanexcitationenergy;//nb: -1 means there was no ion object
    int32_t m_state;//0: undefined, 1: solid, 2: liquid, 3: gas
    struct ElementInfo {
      double fraction;
      EvtFile::index_type index;
      const Element * object;
    };
    mutable std::vector<ElementInfo> m_elements;
  };

}

#include "GriffDataRead/Material.icc"

#endif
