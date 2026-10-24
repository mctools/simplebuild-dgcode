#include "G4Interfaces/ParticleGenToPy.hh"
#include "FlexGen.hh"

SBLDPY_MODULE( _FlexGen ) { G4Interfaces::genToPy<FlexGen>( m ); }
