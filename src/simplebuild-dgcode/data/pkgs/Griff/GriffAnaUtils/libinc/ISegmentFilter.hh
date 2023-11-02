#ifndef GriffAnaUtils_ISegmentFilter_hh
#define GriffAnaUtils_ISegmentFilter_hh

#include "GriffAnaUtils/IFilter.hh"
#include "GriffDataRead/Segment.hh"

namespace GriffAnaUtils {

  class ITrackFilter;

  class ISegmentFilter : public IFilter {
  public:
    //NOTE: Destructor of derived classes must not be public!
    virtual bool filter(const GriffDataRead::Segment*) const = 0;

    //Mainly for the iterators (i.e. a stepfilter looking for energy-depositions
    //> threshold can skip all segments with energy-depositions > threshold
    //instead of investigating the individual steps):
    virtual ITrackFilter* associatedTrackFilter() { return 0; }
  };
}

#endif
