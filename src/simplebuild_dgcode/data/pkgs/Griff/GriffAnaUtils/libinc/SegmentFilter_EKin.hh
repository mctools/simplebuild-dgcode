#ifndef GriffAnaUtils_SegmentFilter_EKin_hh
#define GriffAnaUtils_SegmentFilter_EKin_hh

#include "GriffAnaUtils/ISegmentFilter.hh"
#include <limits>

//Selects segments with post/pre kinetic energy in specified range

namespace GriffAnaUtils {

  class SegmentFilter_EKin : public ISegmentFilter {
  public:

    SegmentFilter_EKin();

    SegmentFilter_EKin* setMinStartEKin(double e);
    SegmentFilter_EKin* setMaxStartEKin(double e);
    SegmentFilter_EKin* setMinEndEKin(double e);
    SegmentFilter_EKin* setMaxEndEKin(double e);

    bool filter(const GriffDataRead::Segment*step) const;

  private:
    virtual ~SegmentFilter_EKin();
    double m_minStartEKin;
    double m_maxStartEKin;
    double m_minEndEKin;
    double m_maxEndEKin;
  };
}

#include "GriffAnaUtils/SegmentFilter_EKin.icc"

#endif
