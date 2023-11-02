#include "G4Interfaces/ParticleGenPyExport.hh"
#include "FlexGen.hh"

PYTHON_MODULE( mod )
{
  ParticleGenPyExport::exportGen<FlexGen>(mod, "FlexGen");
}
