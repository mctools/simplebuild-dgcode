#include "G4Interfaces/StepFilterToPy.hh"
#include "G4CollectFilters/StepFilterTime.hh"

SBLDPY_MODULE( _StepFilterTime ) { G4Interfaces::filterToPy<StepFilterTime>( m ); }
