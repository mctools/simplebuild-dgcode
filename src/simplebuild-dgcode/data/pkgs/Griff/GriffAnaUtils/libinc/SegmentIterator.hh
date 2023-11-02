#ifndef GriffAnaUtils_SegmentIterator_hh
#define GriffAnaUtils_SegmentIterator_hh

#include "GriffDataRead/Segment.hh"
#include "GriffAnaUtils/TrackIterator.hh"
#include "GriffAnaUtils/ISegmentFilter.hh"

//For looping through all segments in an event. Optionally track and segment
//filters can be registered to select what segments to provide. The segment
//filters will only be invoked for segments on tracks which passes all track
//filters.

namespace GriffAnaUtils {

  class SegmentIterator {

  public:

    SegmentIterator(GriffDataReader*);
    ~SegmentIterator();

    ITrackFilter* addFilter(ITrackFilter*);
    ISegmentFilter* addFilter(ISegmentFilter*);

    void reset();//call before iteration (automatically called when the data reader proceeds to a new event)
    const GriffDataRead::Segment* next();//iteration, returns 0 when there are no more segments passing the cuts

  private:
    bool segmentOK(const GriffDataRead::Segment*) const;
    void dereg() { m_dr = 0; }
    std::vector<const ISegmentFilter*> m_segmentFilters;
    TrackIterator m_trkIter;
    const GriffDataRead::Segment* m_itSegment;
    const GriffDataRead::Segment* m_itSegmentEnd;
    struct BegEvtCB : public GriffDataRead::BeginEventCallBack {
      BegEvtCB(SegmentIterator*si) : m_si(si) {}
      virtual void beginEvent(const GriffDataReader*) { m_si->reset(); }
      virtual void dereg(const GriffDataReader*) { m_si->dereg(); }
      SegmentIterator* m_si;
    } m_beginEventCB;
    GriffDataReader * m_dr;
  };

}

#include "GriffAnaUtils/SegmentIterator.icc"

#endif
