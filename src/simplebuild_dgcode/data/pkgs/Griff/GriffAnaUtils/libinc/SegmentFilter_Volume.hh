#ifndef GriffAnaUtils_SegmentFilter_Volume_hh
#define GriffAnaUtils_SegmentFilter_Volume_hh

#include "GriffAnaUtils/ISegmentFilter.hh"
#include <string>

//A simply volume filter which requires an exact match of the
//volumeName. Optionally motherVolumeName/grandMotherVolumeName can be specified
//as well. Volume CopyNumbers are ignored by this filter.
//
//By default, LOGICAL volume names are considered. If instead you wish to select
//based on PHYSICAL volume names, use the "setPhysical() method.

namespace GriffAnaUtils {

  class SegmentFilter_Volume : public ISegmentFilter {
  public:

    SegmentFilter_Volume(const char* volumeName, const char* motherVolumeName=0, const char* grandMotherVolumeName=0);

    SegmentFilter_Volume * setPhysical();//volume names are from the physical volumes
    SegmentFilter_Volume * setLogical();//volume names are from the logical volumes [DEFAULT]

    bool filter(const GriffDataRead::Segment*segment) const;

    bool isPhysical() const;
    bool isLogical() const;

  private:
    virtual ~SegmentFilter_Volume(){}
    unsigned m_depth;
    std::string m_volname[3];
    bool m_logical;
  };
}

#include "GriffAnaUtils/SegmentFilter_Volume.icc"

#endif
