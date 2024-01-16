#ifndef GriffAnaUtils_StepFilter_EnergyDeposition_hh
#define GriffAnaUtils_StepFilter_EnergyDeposition_hh

#include "GriffAnaUtils/IStepFilter.hh"
#include "GriffAnaUtils/SegmentFilter_EnergyDeposition.hh"

//Selects steps with eDep() > threshold

namespace GriffAnaUtils {

  class StepFilter_EnergyDeposition : public IStepFilter {
  public:

    StepFilter_EnergyDeposition(const double& threshold);

    bool filter(const GriffDataRead::Step*step) const;

    //can skip entire segments which do not have eDep() > threshold
    virtual ISegmentFilter* associatedSegmentFilter();

  private:
    virtual ~StepFilter_EnergyDeposition();
    float m_threshold;
    ISegmentFilter * m_segmFilter;
  };
}

#include "GriffAnaUtils/StepFilter_EnergyDeposition.icc"

#endif
