#ifndef GriffDataRead_Step_hh
#define GriffDataRead_Step_hh

#include "Core/Types.hh"
#include <cmath>
#include <string>

namespace GriffDataRead {

  class Segment;
  class Track;

  class Step {

  public:

    //Navigation:
    const Track * getTrack() const;
    const Segment * getSegment() const;
    unsigned iStep() const;//the position of the step within the segment
    const Step * getNextStep() const;//next step within segment (0 if at last)
    const Step * getPreviousStep() const;//previous step within segment (0 if at first)

    //Properties:
    double eDep() const;
    double eDepNonIonising() const;
    double stepLength() const;//can be larger than |globpos_post - globpos_pre|

    double preTime() const;
    double postTime() const;

    double preEKin() const;
    double postEKin() const;

    double preGlobalX() const;
    double preGlobalY() const;
    double preGlobalZ() const;
    double postGlobalX() const;
    double postGlobalY() const;
    double postGlobalZ() const;

    double preLocalX() const;
    double preLocalY() const;
    double preLocalZ() const;
    double postLocalX() const;
    double postLocalY() const;
    double postLocalZ() const;

    bool preAtVolEdge() const;
    bool postAtVolEdge() const;

    double preMomentumX() const;
    double preMomentumY() const;
    double preMomentumZ() const;
    double postMomentumX() const;
    double postMomentumY() const;
    double postMomentumZ() const;

    //efficency/convenience: returns pointer to double[3]/float[3] of coordinates:
    const double* preGlobalArray() const;
    const double* postGlobalArray() const;
    const float* preLocalArray() const;
    const float* postLocalArray() const;
    const float* preMomentumArray() const;
    const float* postMomentumArray() const;

    const std::string& preProcessDefinedStep() const;
    const std::string& postProcessDefinedStep() const;
    const char * preProcessDefinedStepCStr() const;
    const char * postProcessDefinedStepCStr() const;

    //similar enums to in Geant4's G4StepStatus.hh:
    enum STATUS { STATUS_WorldBoundary = 0,
                  STATUS_GeomBoundary = 1,
                  STATUS_AtRestDoItProc = 2,
                  STATUS_AlongStepDoItProc = 3,
                  STATUS_PostStepDoItProc = 4,
                  STATUS_UserDefinedLimit = 5,
                  STATUS_ExclusivelyForcedProc = 6,
                  STATUS_Undefined = 7 };

    STATUS stepStatus() const;
    const std::string& stepStatusStr() const;
    const char* stepStatusCStr() const;

  private:
    std::uint32_t stepStatus_raw() const;
    const Segment * m_segment;
    const char * m_data;
    friend class Segment;
    void set(const Segment *s,const char *d);
  };
}

#ifndef GriffDataRead_Segment_hh
#include "GriffDataRead/Segment.hh"
#endif
#ifndef GriffDataRead_Track_hh
#include "GriffDataRead/Track.hh"
#endif
#ifndef GriffDataRead_GriffDataReader_hh
#include "GriffDataRead/GriffDataReader.hh"
#endif
#include "GriffDataRead/Step.icc"
#endif
