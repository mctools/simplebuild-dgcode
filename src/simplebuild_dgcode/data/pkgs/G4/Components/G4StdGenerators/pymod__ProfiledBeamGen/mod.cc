#include "G4Interfaces/ParticleGenToPy.hh"
#include "G4StdGenerators/ProfiledBeamGen.hh"

SBLDPY_MODULE( _ProfiledBeamGen ) { G4Interfaces::genToPy<ProfiledBeamGen>( m ); }
