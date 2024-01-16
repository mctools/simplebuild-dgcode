#ifndef GriffDataRead_Segment_hh
#define GriffDataRead_Segment_hh

#include "EvtFile/Defs.hh"
#include "Utils/ByteStream.hh"
#include "GriffDataRead/Touchable.hh"
#include "GriffDataRead/Material.hh"

class GriffDataReader;

namespace GriffDataRead {

  class Track;
  class Step;

  class Segment {
  public:
    //Navigation:

    const Track* getTrack() const;
    unsigned iSegment() const;//the position of the segment on the track
    const Segment * getNextSegment() const;//returns 0 if this is the last segment.
    const Segment * getPreviousSegment() const;//returns 0 if this is the first segment.
    bool nextWasFiltered() const;//returns true if first step coming after this segment was filtered out.

    //WARNING: Calling any of the next methods will usually trigger loading of all step data in the event!
    unsigned nStepsOriginal() const;//number of steps comprising the segment
    unsigned nStepsStored() const;//number of steps actually available in the inputfile (in FULL mode nStepsOriginal() == nStepsStored())

    bool hasStepInfo() const { return nStepsStored() > 0; }
    const Step * getStep(unsigned i) const;//i must be less than nStepsStored()
    const Step * stepBegin() const;
    const Step * stepEnd() const;
    const Step * firstStep() const;
    const Step * lastStep() const;
    double segmentLength() const;//loads steps and sums their stepLength()'s.

    //Properties:

    double startTime() const;
    double endTime() const;

    double startEKin() const;
    double endEKin() const;

    float eDep() const;
    float eDepNonIonising() const;

    bool startAtVolumeBoundary() const;
    bool endAtVolumeBoundary() const;

    unsigned volumeDepthStored() const;//Number of "generations" of volumes which can be querried.
    const std::string& volumeName(unsigned idepth=0) const;//i=1 for mother volume, i=2 for grand-mother volume.
    const char* volumeNameCStr(unsigned idepth=0) const;//same
    const std::string& physicalVolumeName(unsigned idepth=0) const;//i=1 for mother volume, i=2 for grand-mother volume.
    const char* physicalVolumeNameCStr(unsigned idepth=0) const;//same
    int volumeCopyNumber(unsigned idepth=0) const;//i=1 for mother volume, i=2 for grand-mother volume.
    bool isInWorldVolume() const;
    const Material* material(unsigned idepth=0) const;//i=1 for mother volume, i=2 for grand-mother volume.

    bool inSameVolume(const Segment*) const;//see if two segments are in the same volume

    //////////////////////////////////////////////////////
    //  The rest of this file is implementation details //
    //////////////////////////////////////////////////////

  private:
    const Touchable& getTouchable() const;
    int32_t rawStepIdx() const;
    void setupSteps() const;
    void actualSetupSteps() const;

    friend class Track;
    friend class ::GriffDataReader;

    Segment(Track*trk,const char* data) : m_trk(trk), m_data(data), m_stepsBegin(0), m_stepsEnd(0) {}
    ~Segment(){}
    mutable Track * m_trk;
    const char* m_data;
    friend class Step;
    mutable const Step* m_stepsBegin;
    mutable const Step* m_stepsEnd;//0 means steps are not setup, 0x1 means the mode is minimal.
    EvtFile::index_type touchableIndex() const;
  };
}

#ifndef GriffDataRead_GriffDataReader_hh
#include "GriffDataRead/GriffDataReader.hh"
#endif
#ifndef GriffDataRead_Track_hh
#include "GriffDataRead/Track.hh"
#endif
#ifndef GriffDataRead_Step_hh
#include "GriffDataRead/Step.hh"
#endif
#include "GriffDataRead/Segment.icc"
#endif
