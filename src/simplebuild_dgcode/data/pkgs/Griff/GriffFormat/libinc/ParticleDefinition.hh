#ifndef GriffFormat_ParticleDefinition_hh
#define GriffFormat_ParticleDefinition_hh

#include "EvtFile/Defs.hh"
#include <cassert>
#include <cstring>

namespace EvtFile {
  class FileWriter;
}

namespace GriffFormat {

  //Object which can be streamed. We ensure no padding by keeping
  //sizeof(ParticleDefinition) as a multiple of 8 and by careful arrangement of
  //datamembers. This is checked compile-time in the dummy method below.


  struct ParticleDefinition {
    ParticleDefinition(){}//no init, caller inits all variables afterwards
    ParticleDefinition(const char*&data);//init from stream of data
    void write(EvtFile::FileWriter&fw);//stream already initialised object to fw

    double mass;
    double width;
    double charge;
    double lifeTime;
    EvtFile::index_type nameIdx;   //indice for string database
    EvtFile::index_type typeIdx;   // --||--
    EvtFile::index_type subTypeIdx;// --||--
    int32_t pdgcode;
    int32_t atomicNumber;
    int32_t atomicMass;
    float magneticMoment;
    int16_t spin_halfs;// spin in units of 1/2.
    double spin() const { return spin_halfs*0.5; }
    std::uint8_t stable;
    std::uint8_t shortLived;

    bool operator==(const ParticleDefinition&o) const
    {
      return 0 == std::memcmp((void*)(this),(void*)(&o),sizeof(*this));
    }
    bool operator!=(const ParticleDefinition&o) const
    {
      return !(*this == o);
    }
  private:
    void dummy()
    {
      const unsigned PERSISTENT_SIZE = 64;
      //Double-check assumptions regarding data size and padding:
      static_assert(sizeof(float)==4);
      static_assert(sizeof(double)==8);
      static_assert(sizeof(EvtFile::index_type)==4);
      static_assert(sizeof(*this)==PERSISTENT_SIZE);
    }
  public:
    void writeRaw(char* data);//For test, don't use!
  };

}

#endif
