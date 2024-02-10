/////////////////////////////////////////
// Declaration of our geometry module: //
/////////////////////////////////////////

#include "G4Interfaces/GeoConstructPyExport.hh"

class GeoEmptyWorld final : public G4Interfaces::GeoConstructBase
{
public:
  GeoEmptyWorld();
  virtual ~GeoEmptyWorld(){}
  G4VPhysicalVolume* Construct() override;
};

PYTHON_MODULE( mod ) { GeoConstructPyExport::exportGeo<GeoEmptyWorld>(mod, "GeoEmptyWorld"); }

////////////////////////////////////////////
// Implementation of our geometry module: //
////////////////////////////////////////////

#include "G4Box.hh"

GeoEmptyWorld::GeoEmptyWorld()
  : GeoConstructBase("G4StdGeometries/GeoEmptyWorld")
{
  addParameterString("material","Vacuum");
  addParameterDouble("dimension_cm",100);
}

G4VPhysicalVolume* GeoEmptyWorld::Construct()
{
  double d = getParameterDouble("dimension_cm")*CLHEP::cm;
  auto world = place(new G4Box("World",d,d,d),
                     getParameterMaterial("material"),0,0,0,0,INVISIBLE);
  return world.physvol;
}
