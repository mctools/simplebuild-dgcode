#ifndef GriffAnaUtils_SegmentFilter_Time_hh
#define GriffAnaUtils_SegmentFilter_Time_hh

#include "GriffAnaUtils/ISegmentFilter.hh"
#include <limits>

//Selects segments based on their time stamps. Either by demanding that they
//START or ENDS BEFORE or AFTER the specified time coordinate, or by demanding
//that the specified time coordinate is CONTAINed in the time span given by the
//start and end of the segment.

namespace GriffAnaUtils {

  enum TIME_FILTER_MODE { STARTS_BEFORE, STARTS_AFTER, ENDS_BEFORE, ENDS_AFTER, CONTAINS };

  class SegmentFilter_Time : public ISegmentFilter {
  public:

    SegmentFilter_Time(const double& time, TIME_FILTER_MODE m = CONTAINS );

    bool filter(const GriffDataRead::Segment*segment) const;

  private:
    virtual ~SegmentFilter_Time(){}
    double m_slt;
    double m_sgt;
    double m_egt;
    double m_elt;
  };
}

#include "GriffAnaUtils/SegmentFilter_Time.icc"

#endif
