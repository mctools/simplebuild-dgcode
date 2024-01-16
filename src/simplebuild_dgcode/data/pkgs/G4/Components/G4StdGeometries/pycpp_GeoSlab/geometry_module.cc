/////////////////////////////////////////
// Declaration of our geometry module: //
/////////////////////////////////////////

#include "G4Interfaces/GeoConstructPyExport.hh"

class GeoSlab : public G4Interfaces::GeoConstructBase
{
public:
  GeoSlab();
  virtual ~GeoSlab(){}
  G4VPhysicalVolume* Construct();
};

PYTHON_MODULE( mod ) { GeoConstructPyExport::exportGeo<GeoSlab>(mod,"GeoSlab"); }

////////////////////////////////////////////
// Implementation of our geometry module: //
////////////////////////////////////////////

#include "G4Box.hh"
#include "G4Orb.hh"

GeoSlab::GeoSlab()
  : GeoConstructBase("G4StdGeometries/GeoSlab")
{
  addParameterDouble("target_depth_cm",0.01);
  addParameterDouble("target_width_cm",100.0);
  addParameterString("material","G4_POLYETHYLENE");
}

G4VPhysicalVolume* GeoSlab::Construct()
{
  //Place target box at positive Z, and identical (for simplicity) recording box at negative Z

  const double target_depth = getParameterDouble("target_depth_cm")*Units::cm;
  const double target_width = getParameterDouble("target_width_cm")*Units::cm;
  auto mat = getParameterMaterial("material");
  auto mat_vacuum = getMaterial("Vacuum");

  //World is slightly larger than contained volumes (so the world can be used as a recording volume in all directions):
  auto worldvols = place(new G4Box("World", 0.51*target_width, 0.51*target_width, 21*target_depth),
                         mat_vacuum,0,0,0,0,INVISIBLE);
  auto lvWorld = worldvols.logvol;
  auto pvWorld = worldvols.physvol;

  place(new G4Box("Target",0.5*target_width,0.5*target_width,0.5*target_depth),
        mat,0,0,+0.5*target_depth,lvWorld,G4Colour(1.0, 0.0, 0.0));

  place(new G4Box("RecordBack",0.5*target_width,0.5*target_width,0.5*target_depth),
        mat_vacuum,0,0,-0.5*target_depth,lvWorld,G4Colour(0.0, 0.0, 1.0));

  place(new G4Box("RecordFwd",0.5*target_width,0.5*target_width,0.5*target_depth),
        mat_vacuum,0,0,1.5*target_depth,lvWorld,G4Colour(0.0, 0.0, 1.0));

  return pvWorld;
}
