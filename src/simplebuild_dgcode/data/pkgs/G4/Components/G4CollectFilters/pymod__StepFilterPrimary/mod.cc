#include "G4Interfaces/StepFilterToPy.hh"
#include "G4CollectFilters/StepFilterPrimary.hh"

SBLDPY_MODULE( _StepFilterPrimary ) { G4Interfaces::filterToPy<StepFilterPrimary>( m ); }
