#include "G4Interfaces/StepFilterPyExport.hh"
#include "G4CollectFilters/StepFilterVolume.hh"

PYTHON_MODULE( mod )
{
  StepFilterPyExport::exportFilter<StepFilterVolume>(mod, "StepFilterVolume");
}
