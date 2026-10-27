//Test that "using CLHEP::m;" etc. from G4SystemOfUnits.hh does not trigger
//-Wshadow warnings in the headers below or in SBLDPY_MODULE:
#include "G4SystemOfUnits.hh"
#include "G4Interfaces/GeoConstructToPy.hh"
#include "G4Tests/GeoTest.hh"

SBLDPY_MODULE( _GeoTest )
{
  G4Interfaces::geoToPy<G4Tests::GeoTest>( m )
    .def( "worldVolume_m3", &G4Tests::GeoTest::worldVolume_m3, "World volume in cubic meters" )
    .def( "label", &G4Tests::GeoTest::label, { "prefix", "n", "x" } )
    .def( "incrementCounter", &G4Tests::GeoTest::incrementCounter, { "step" } );
}
