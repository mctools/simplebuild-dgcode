#ifndef GriffAnaUtils_SegmentFilter_EnergyDeposition_hh
#define GriffAnaUtils_SegmentFilter_EnergyDeposition_hh

#include "GriffAnaUtils/ISegmentFilter.hh"

//Selects segments with eDep() > threshold

namespace GriffAnaUtils {

  class SegmentFilter_EnergyDeposition : public ISegmentFilter {
  public:

    SegmentFilter_EnergyDeposition(const double& threshold);

    bool filter(const GriffDataRead::Segment*segment) const;

  private:
    virtual ~SegmentFilter_EnergyDeposition(){}
    float m_threshold;
  };
}

#include "GriffAnaUtils/SegmentFilter_EnergyDeposition.icc"

#endif
