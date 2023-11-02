#ifndef GriffAnaUtils_StepFilter_EKin_hh
#define GriffAnaUtils_StepFilter_EKin_hh

#include "GriffAnaUtils/IStepFilter.hh"
#include "GriffAnaUtils/SegmentFilter_EKin.hh"
#include <limits>

//Selects steps with post/pre kinetic energy in specified range

namespace GriffAnaUtils {

  class StepFilter_EKin : public IStepFilter {
  public:

    StepFilter_EKin();

    StepFilter_EKin* setMinPreEKin(double e);
    StepFilter_EKin* setMaxPreEKin(double e);
    StepFilter_EKin* setMinPostEKin(double e);
    StepFilter_EKin* setMaxPostEKin(double e);

    bool filter(const GriffDataRead::Step*step) const;

  private:
    virtual ~StepFilter_EKin();
    double m_minPreEKin;
    double m_maxPreEKin;
    double m_minPostEKin;
    double m_maxPostEKin;
  };
}

#include "GriffAnaUtils/StepFilter_EKin.icc"

#endif
