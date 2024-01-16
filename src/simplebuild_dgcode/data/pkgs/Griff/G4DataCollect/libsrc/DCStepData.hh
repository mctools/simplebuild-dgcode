#ifndef DCStepData_hh
#define DCStepData_hh
#include "Core/Types.hh"
#include "DCMgr.hh"
#include "G4String.hh"
#include "DBTouchableEntry.hh"
#include <cmath>
class G4Step;
class G4StepPoint;

//todo: polarisation? fpProcessDefinedStep?

namespace G4DataCollectInternals {

  struct DCStepData
  {
    //NB: Due to lack of a proper interface in Geant4.5 we do *not* keep track
    //of which daughter particles were created in which step.

    struct EndPointData
    {
      //Details only available in the full step info section:
      double globpos[3];
      float locpos[3];
      float mom[3];
      float weight;
      double time;//global since event gen
      double eKin;//etot = mass*c^2+ekin
      EvtFile::index_type processDefiningStepIdx;
      bool atVolEdge;
      void set(G4StepPoint*,const G4AffineTransform& topTransform,DCMgr&mgr);
      void write(EvtFile::FileWriter&fw)
      {
        //weight is stored in the track section, not here. |eKin| and atVolEdge
        //are stored in the same double, by using the sign bit for atVolEdge.

        //encode |eKin| and atVolEdge, taking care when |eKin|==0
        double eKinAndVolEdge;
        if (std::fabs(eKin))
          eKinAndVolEdge = std::fabs(eKin)*(atVolEdge?-1:1);
        else {
          eKinAndVolEdge = atVolEdge ? double(-0.0) : double(0.0);
        }
        assert( std::fabs(eKin) == std::fabs(eKinAndVolEdge) );
        assert( atVolEdge == bool(std::signbit(eKinAndVolEdge)) );

#ifndef NDEBUG
        unsigned stored_before = fw.sizeFullDataSection();
#endif
        fw.writeDataFullSection(globpos);//3 doubles
        static_assert(sizeof(globpos)==3*sizeof(double));
        fw.writeDataFullSection(time);//1 double
        fw.writeDataFullSection(eKinAndVolEdge);//1 double
        fw.writeDataFullSection(locpos);//3 floats
        static_assert(sizeof(locpos)==3*sizeof(float));
        fw.writeDataFullSection(mom);//3 floats
        static_assert(sizeof(mom)==3*sizeof(float));
        fw.writeDataFullSection(processDefiningStepIdx);//1 EvtFile::index_type (4 bytes)
        static_assert(GriffFormat::Format::SIZE_STEPPREPOSTPART==5*sizeof(double)+6*sizeof(float)+sizeof(EvtFile::index_type));
        assert(fw.sizeFullDataSection()-stored_before==GriffFormat::Format::SIZE_STEPPREPOSTPART);
      }

    };
    EndPointData preStep;
    EndPointData postStep;

    //Track-object variables:
    G4int trkId;
    G4int parentId;
    G4int stepNbr;
    G4int pdgcode;
    const G4String* creatorProcessName;//only set when stepNbr<2

    //along-step data:
    double eDep;//persistified as float, but kept in double-precision until then
    double eDepNonIonizing;//persistified as float, but kept in double-precision until then
    double stepLength;//persistified as float, but kept in double-precision until then
    std::uint32_t stepStatus; //actually 4 bits would be enough, but no other step data can use the 3.5 leftover bytes

    DBTouchableEntry * touchableEntry;//only set when step is likely to be in a
                                      //different volume from previous step (and
                                      //technically the touchable is taken from
                                      //the prestep, but we still consider it to
                                      //also be valid for the along-step).
    G4VPhysicalVolume* prestepVolPtr;//used to cross-check and correct G4's atpoststep/atprestep flags.
    EvtFile::index_type volIdx;//used during post-processing to cache the volume index.

    unsigned segmentEnd;//used during post-processing to mark the start of a
                        //segment and provide a pointer to the end (+1) of
                        //the segment.

    //NB: Do *not* init all variables here (for efficiency), only in ::set(..)).:
    bool valid() const { return stepNbr!=-99; }
    DCStepData() : stepNbr(-99),touchableEntry(0),segmentEnd(EvtFile::INDEX_MAX) { assert(!valid());}
    void clear()
    {
#ifdef GRIFF_EXTRA_TESTS
      assert(valid());
      extraTests();
#endif
      if (touchableEntry)
        touchableEntry->unref();
      touchableEntry=0;
      stepNbr = -99;
      segmentEnd=EvtFile::INDEX_MAX;
#ifdef GRIFF_EXTRA_TESTS
      assert(!valid());
#endif
    }
    ~DCStepData()
    {
#ifdef GRIFF_EXTRA_TESTS
      assert(!valid());//should already have been cleared
#endif
    }

    void set(const G4Step*,DCMgr&,int isNewVolOnSameTrack,G4DataCollectInternals::DCStepData*);//must init all vars
#ifdef GRIFF_EXTRA_TESTS
    void extraTests() const { if (touchableEntry) touchableEntry->extraTests(); }
#endif
  private:
    //explicitely forbid inadvertant copy/assignment:
    DCStepData( const DCStepData & );
    DCStepData & operator= ( const DCStepData & );

  };
}

#endif
