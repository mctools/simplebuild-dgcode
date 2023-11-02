#ifndef GriffAnaUtils_TrackFilter_Primary_hh
#define GriffAnaUtils_TrackFilter_Primary_hh

#include "GriffAnaUtils/ITrackFilter.hh"
#include "Utils/FastLookupSet.hh"
#include "Core/Types.hh"
#include <set>

//For selecting primary particles only (obviously, negate it to select secondary
//particles only).

namespace GriffAnaUtils {

  class TrackFilter_Primary : public ITrackFilter {
  public:
    TrackFilter_Primary() { }

    bool filter(const GriffDataRead::Track*trk) const
    {
      return trk->isPrimary();
    }

  private:
    TrackFilter_Primary( const TrackFilter_Primary & );
    TrackFilter_Primary & operator= ( const TrackFilter_Primary & );
    virtual ~TrackFilter_Primary(){}
  };
}

#endif
