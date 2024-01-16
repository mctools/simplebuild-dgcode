#include "G4Interfaces/ParticleGenPyExport.hh"
#include "G4StdGenerators/SimpleGen.hh"

PYTHON_MODULE( mod )
{
  ParticleGenPyExport::exportGen<SimpleGen>(mod, "SimpleGen");
}
