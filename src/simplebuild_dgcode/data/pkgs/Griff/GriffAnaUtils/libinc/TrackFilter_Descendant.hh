#ifndef GriffAnaUtils_TrackFilter_Descendant_hh
#define GriffAnaUtils_TrackFilter_Descendant_hh

#include "GriffAnaUtils/ITrackFilter.hh"

namespace GriffAnaUtils {

  class TrackFilter_Descendant : public ITrackFilter {
  public:
    TrackFilter_Descendant();

    //Must be called each event before the filter (or any iterators it is part
    //of) is used:
    void setAncestor(const GriffDataRead::Track*,bool includeAncestor = true);

    bool filter(const GriffDataRead::Track*trk) const;

  private:
    TrackFilter_Descendant( const TrackFilter_Descendant & );
    TrackFilter_Descendant & operator= ( const TrackFilter_Descendant & );
    virtual ~TrackFilter_Descendant(){}
    const GriffDataRead::Track* m_ancestor;
#ifndef NDEBUG
    double m_ancestorStartTime;//for cross-check that setAncestor is not forgotten
#endif
    bool m_includeAncestor;
  };
}

#include "GriffAnaUtils/TrackFilter_Descendant.icc"

#endif
