#ifndef GriffAnaUtils_TrackIterator_hh
#define GriffAnaUtils_TrackIterator_hh

#include "GriffDataRead/Track.hh"
#include "GriffAnaUtils/ITrackFilter.hh"

//For looping through all tracks in an event. Optionally track filters can be
//registered to select what tracks to provide.

namespace GriffAnaUtils {

  class TrackIterator {

  public:

    TrackIterator(GriffDataReader*);
    ~TrackIterator();

    ITrackFilter* addFilter(ITrackFilter*);

    void reset();//call before iteration (automatically called when the data reader proceeds to a new event)
    const GriffDataRead::Track* next();//iteration, returns 0 when there are no more tracks passing the cuts

  private:
    bool trackOK(const GriffDataRead::Track*) const;
    void dereg() { m_dr = 0; }
    std::vector<const ITrackFilter*> m_trackFilters;
    GriffDataReader* m_dr;
    const GriffDataRead::Track* m_itTrack;
    const GriffDataRead::Track* m_itTrackEnd;
    struct BegEvtCB : public GriffDataRead::BeginEventCallBack {
      BegEvtCB(TrackIterator*ti) : m_ti(ti) {}
      virtual void beginEvent(const GriffDataReader*) { m_ti->reset(); }
      virtual void dereg(const GriffDataReader*) { m_ti->dereg(); }
      TrackIterator* m_ti;
    } m_beginEventCB;
  };
}


#include "GriffAnaUtils/TrackIterator.icc"

#endif
