#include "G4Interfaces/ParticleGenPyExport.hh"
#include "G4Launcher/SingleParticleGun.hh"

PYTHON_MODULE( mod )
{
  ParticleGenPyExport::exportGen<G4Launcher::SingleParticleGun>(mod,"SingleParticleGun");
}
