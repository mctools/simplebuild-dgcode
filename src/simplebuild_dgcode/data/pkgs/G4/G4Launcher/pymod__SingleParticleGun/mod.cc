#include "G4Interfaces/ParticleGenToPy.hh"
#include "G4Launcher/SingleParticleGun.hh"

SBLDPY_MODULE( _SingleParticleGun ) { G4Interfaces::genToPy<G4Launcher::SingleParticleGun>( m ); }
