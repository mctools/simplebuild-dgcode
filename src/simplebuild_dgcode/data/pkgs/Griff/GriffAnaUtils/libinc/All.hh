#ifndef GriffAnaUtils_All_hh
#define GriffAnaUtils_All_hh

//Convenience header which includes all filters and iterators from the
//GriffAnaUtils package and the GriffDataReader header.

#include "GriffAnaUtils/TrackIterator.hh"
#include "GriffAnaUtils/SegmentIterator.hh"
#include "GriffAnaUtils/StepIterator.hh"

#include "GriffAnaUtils/TrackFilter_Charged.hh"
#include "GriffAnaUtils/TrackFilter_Descendant.hh"
#include "GriffAnaUtils/TrackFilter_PDGCode.hh"
#include "GriffAnaUtils/TrackFilter_Primary.hh"

#include "GriffAnaUtils/SegmentFilter_EKin.hh"
#include "GriffAnaUtils/SegmentFilter_EnergyDeposition.hh"
#include "GriffAnaUtils/SegmentFilter_Time.hh"
#include "GriffAnaUtils/SegmentFilter_Volume.hh"

#include "GriffAnaUtils/StepFilter_EKin.hh"
#include "GriffAnaUtils/StepFilter_EnergyDeposition.hh"
#include "GriffAnaUtils/StepFilter_Time.hh"

#include "GriffDataRead/GriffDataReader.hh"
#include "Units/Units.hh"

#endif
