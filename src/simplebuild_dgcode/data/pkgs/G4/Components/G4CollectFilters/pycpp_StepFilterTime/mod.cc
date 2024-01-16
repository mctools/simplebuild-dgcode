#include "G4Interfaces/StepFilterPyExport.hh"
#include "G4CollectFilters/StepFilterTime.hh"

PYTHON_MODULE( mod )
{
  StepFilterPyExport::exportFilter<StepFilterTime>(mod, "StepFilterTime");
}
