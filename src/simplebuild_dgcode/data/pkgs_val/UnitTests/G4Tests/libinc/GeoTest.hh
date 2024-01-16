#include "G4Interfaces/GeoConstructBase.hh"

namespace G4Tests {

  struct GeoTest : public G4Interfaces::GeoConstructBase {
    GeoTest() : GeoConstructBase("G4Tests/GeoTest")
    {
      addParameterDouble("boronThickness_micron",1.0,0.01,1000.0);
      addParameterDouble("worldExtent_meters",1.0,0.01,1000.0);
    }
    G4VPhysicalVolume* Construct();
  };

}
