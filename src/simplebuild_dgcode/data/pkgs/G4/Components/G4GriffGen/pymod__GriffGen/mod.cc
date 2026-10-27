#include "G4Interfaces/ParticleGenToPy.hh"
#include "GriffGen.hh"

SBLDPY_MODULE( _GriffGen ) { G4Interfaces::genToPy<GriffGen>( m ); }
