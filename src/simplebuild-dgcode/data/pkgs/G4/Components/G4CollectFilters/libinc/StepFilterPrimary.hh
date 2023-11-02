#ifndef G4DataCollect_StepFilterPrimary_hh
#define G4DataCollect_StepFilterPrimary_hh

//filter for picking out steps solely from primary particles.

#include "G4Interfaces/StepFilterBase.hh"

class StepFilterPrimary : public G4Interfaces::StepFilterBase {
public:
  StepFilterPrimary();
  virtual ~StepFilterPrimary(){}
  virtual bool filterStep(const G4Step*step) const;
};

#endif
