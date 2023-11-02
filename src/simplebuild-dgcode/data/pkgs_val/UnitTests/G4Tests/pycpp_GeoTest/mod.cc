#include "G4Interfaces/GeoConstructPyExport.hh"
#include "G4Tests/GeoTest.hh"

PYTHON_MODULE( mod )
{
  GeoConstructPyExport::exportGeo<G4Tests::GeoTest>(mod, "GeoTest");
}
