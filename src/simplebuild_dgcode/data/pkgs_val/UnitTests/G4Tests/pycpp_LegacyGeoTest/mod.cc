//Module based on the legacy pybind11 based export headers, which are still
//used by external projects. It is only here to test that such modules keep
//working (including in combination with sbldpy based modules, see the
//testtopy script), and should be removed along with those headers.

#include "G4Interfaces/GeoConstructPyExport.hh"
#include "G4Tests/GeoTest.hh"

PYTHON_MODULE( mod )
{
  GeoConstructPyExport::exportGeo<G4Tests::GeoTest>(mod, "LegacyGeoTest");
}
