#ifndef GriffDataRead_Track_hh
#define GriffDataRead_Track_hh

#include "Core/Types.hh"
#include "Utils/ByteStream.hh"
#include "GriffFormat/ParticleDefinition.hh"
#include "EvtFile/Defs.hh"
#include <cassert>
#include <string>

class GriffDataReader;

namespace GriffDataRead
{
  class Segment;
  class Step;

  class Track {
  public:

    //Unique ID:

    int32_t trackID() const;

    //Navigation:

    const GriffDataReader * getDataReader() const;

    int32_t parentID() const;
    const Track * getParent() const;

    std::uint32_t nDaughters() const;
    const Track * getDaughter(unsigned i) const;
    int32_t getDaughterID(unsigned i) const;

    const int32_t * daughterIDBegin() const;
    const int32_t * daughterIDEnd() const;

    std::uint32_t nSegments() const;
    const Segment * getSegment(unsigned i) const;
    const Segment * segmentBegin() const;
    const Segment * segmentEnd() const;
    const Segment * firstSegment() const;
    const Segment * lastSegment() const;

    //Access first and last step on the track directly (note, NOT for iteration
    //since they might be on different segments):
    const Step * firstStep() const;
    const Step * lastStep() const;

    //Track properties in this event:

    float weight() const;
    bool isPrimary() const;
    bool isSecondary() const;

    double startTime() const;
    double startEKin() const;

    const std::string& creatorProcess() const;
    const char* creatorProcessCStr() const;

    //Properties based on pdgCode:

    int32_t pdgCode() const;
    const std::string& pdgName() const;
    const std::string& pdgType() const;
    const std::string& pdgSubType() const;
    const char* pdgNameCStr() const;
    const char* pdgTypeCStr() const;
    const char* pdgSubTypeCStr() const;
    double mass() const;
    double width() const;
    double charge() const;
    double lifeTime() const;
    int32_t atomicNumber() const;
    int32_t atomicMass() const;
    float magneticMoment() const;
    double spin() const;
    bool stable() const;
    bool shortLived() const;

    //////////////////////////////////////////////////////
    //  The rest of this file is implementation details //
    //////////////////////////////////////////////////////

    //for std::vector<Track> only, not actually allowed (asserts false runtime!):
    Track();
    ~Track();
    Track & operator= ( const Track & o );
    Track( const Track & o );
  private:
    mutable ::GriffDataReader * m_dr;
    mutable const GriffFormat::ParticleDefinition * m_pdef;
    const char * m_data;
    const Segment * m_firstSegment;
    friend class ::GriffDataReader;
    friend class Segment;
    friend class Step;
    void set(GriffDataReader*,const char*);
    void lookupPartDef() const;
    const GriffFormat::ParticleDefinition * pdef() const;
  };
}

#ifndef GriffDataRead_Segment_hh
#include "GriffDataRead/Segment.hh"
#endif
#ifndef GriffDataRead_Step_hh
#include "GriffDataRead/Step.hh"
#endif
#ifndef GriffDataRead_GriffDataReader_hh
#include "GriffDataRead/GriffDataReader.hh"
#endif
#include "GriffDataRead/Track.icc"
#endif
