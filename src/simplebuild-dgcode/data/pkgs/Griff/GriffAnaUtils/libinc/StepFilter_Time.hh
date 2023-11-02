#ifndef GriffAnaUtils_StepFilter_Time_hh
#define GriffAnaUtils_StepFilter_Time_hh

#include "GriffAnaUtils/IStepFilter.hh"
#include "GriffAnaUtils/SegmentFilter_Time.hh"

//Selects segments based on their time stamps. Either by demanding that they
//START or ENDS BEFORE or AFTER the specified time coordinate, or by demanding
//that the specified time coordinate is CONTAINed in the time span given by the
//start and end of the segment.

namespace GriffAnaUtils {

  class StepFilter_Time : public IStepFilter {
  public:

    //The TIME_FILTER_MODE enum is defined in SegmentFilter_Time.hh

    StepFilter_Time(const double& time, TIME_FILTER_MODE m = CONTAINS);

    bool filter(const GriffDataRead::Step*step) const;

    //can skip entire segments which do not have eDep() > threshold
    virtual ISegmentFilter* associatedSegmentFilter();

  private:
    virtual ~StepFilter_Time();
    double m_slt;
    double m_sgt;
    double m_egt;
    double m_elt;
    ISegmentFilter * m_segmFilter;
  };
}

#include "GriffAnaUtils/StepFilter_Time.icc"

#endif
