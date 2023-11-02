#ifndef GriffAnaUtils_IStepFilter_hh
#define GriffAnaUtils_IStepFilter_hh

#include "GriffAnaUtils/IFilter.hh"
#include "GriffDataRead/Step.hh"

namespace GriffAnaUtils {

  class ITrackFilter;
  class ISegmentFilter;

  class IStepFilter : public IFilter {
  public:
    //NOTE: Destructor of derived classes must not be public!
    virtual bool filter(const GriffDataRead::Step*) const = 0;

    //Mainly for the iterators (i.e. a stepfilter looking for energy-depositions
    //> threshold can skip all segments with energy-depositions > threshold
    //instead of investigating the individual steps):
    virtual ISegmentFilter* associatedSegmentFilter() { return 0; }
    virtual ITrackFilter* associatedTrackFilter() { return 0; }

  };
}

#endif
