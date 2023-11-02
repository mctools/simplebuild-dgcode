#ifndef GriffAnaUtils_StepIterator_hh
#define GriffAnaUtils_StepIterator_hh

#include "GriffDataRead/Step.hh"
#include "GriffAnaUtils/SegmentIterator.hh"
#include "GriffAnaUtils/IStepFilter.hh"

//For looping through all (stored!) steps in an event. Optionally track, segment
//and step filters can be registered to select what steps to provide. Track
//filters are checked before segment filters which are checked before step
//filters.

namespace GriffAnaUtils {

  class StepIterator {

  public:

    StepIterator(GriffDataReader*);
    ~StepIterator();

    ITrackFilter* addFilter(ITrackFilter*);
    ISegmentFilter* addFilter(ISegmentFilter*);
    IStepFilter* addFilter(IStepFilter*);

    void reset();//call before iteration (automatically called when the data reader proceeds to a new event)
    const GriffDataRead::Step* next();//iteration, returns 0 when there are no more segments passing the cuts

  private:
    bool stepOK(const GriffDataRead::Step*) const;
    void dereg() { m_dr = 0; }
    std::vector<const IStepFilter*> m_stepFilters;
    SegmentIterator m_segmentIter;
    const GriffDataRead::Step* m_itStep;
    const GriffDataRead::Step* m_itStepEnd;
    struct BegEvtCB : public GriffDataRead::BeginEventCallBack {
      BegEvtCB(StepIterator*si) : m_si(si) {}
      virtual void beginEvent(const GriffDataReader*) { m_si->reset(); }
      virtual void dereg(const GriffDataReader*) { m_si->dereg(); }
      StepIterator* m_si;
    } m_beginEventCB;
    GriffDataReader * m_dr;
 };

}

#include "GriffAnaUtils/StepIterator.icc"

#endif
