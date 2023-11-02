#include "G4CollectFilters/StepFilterPrimary.hh"
#include "G4Step.hh"
#include "G4Track.hh"

StepFilterPrimary::StepFilterPrimary()
  : G4Interfaces::StepFilterBase("G4CollectFilters/StepFilterPrimary")
{
}

bool StepFilterPrimary::filterStep(const G4Step*step) const
{
  return step->GetTrack()->GetParentID()==0;
}
