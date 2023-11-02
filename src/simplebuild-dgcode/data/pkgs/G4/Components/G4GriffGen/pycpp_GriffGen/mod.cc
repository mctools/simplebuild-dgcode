#include "G4Interfaces/ParticleGenPyExport.hh"
#include "GriffGen.hh"

PYTHON_MODULE( mod )
{
  ParticleGenPyExport::exportGen<GriffGen>(mod, "GriffGen");
}
