#include "G4Interfaces/StepFilterToPy.hh"
#include "G4CollectFilters/StepFilterVolume.hh"

SBLDPY_MODULE( _StepFilterVolume ) { G4Interfaces::filterToPy<StepFilterVolume>( m ); }
