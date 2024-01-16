//all includes here to make sure Core/Python is last (workaround for osx bug)

#include "GriffAnaUtils/TrackIterator.hh"
#include "GriffAnaUtils/SegmentIterator.hh"
#include "GriffAnaUtils/StepIterator.hh"
#include "GriffAnaUtils/IFilter.hh"
#include "GriffAnaUtils/ITrackFilter.hh"
#include "GriffAnaUtils/TrackFilter_Charged.hh"
#include "GriffAnaUtils/TrackFilter_Primary.hh"
#include "GriffAnaUtils/TrackFilter_Descendant.hh"
#include "GriffAnaUtils/TrackFilter_PDGCode.hh"
#include "GriffAnaUtils/SegmentFilter_Volume.hh"
#include "GriffAnaUtils/SegmentFilter_EnergyDeposition.hh"
#include "GriffAnaUtils/StepFilter_EnergyDeposition.hh"
#include "GriffAnaUtils/SegmentFilter_EKin.hh"
#include "GriffAnaUtils/StepFilter_EKin.hh"
#include "Core/Python.hh"

#include "filters.hh"
#include "iterators.hh"

PYTHON_MODULE( mod )
{
  pyextra::pyimport("GriffDataRead");
  GriffAnaUtils::pyexport_filters(mod);
  GriffAnaUtils::pyexport_iterators(mod);
}
