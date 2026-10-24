#include "G4Interfaces/GeoConstructToPy.hh"
#include "G4Tests/GeoTest.hh"

SBLDPY_MODULE( _GeoTest ) { G4Interfaces::geoToPy<G4Tests::GeoTest>( m ); }
