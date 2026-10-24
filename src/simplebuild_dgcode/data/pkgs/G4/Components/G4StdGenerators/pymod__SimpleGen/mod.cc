#include "G4Interfaces/ParticleGenToPy.hh"
#include "G4StdGenerators/SimpleGen.hh"

SBLDPY_MODULE( _SimpleGen ) { G4Interfaces::genToPy<SimpleGen>( m ); }
