/////////////////////////////////////////
// Declaration of our geometry module: //
/////////////////////////////////////////

#include "G4Interfaces/GeoConstructPyExport.hh"

class GeoSlab final : public G4Interfaces::GeoConstructBase
{
public:
  GeoSlab();
  virtual ~GeoSlab(){}
  G4VPhysicalVolume* Construct() override;
  bool validateParameters() override;
};

PYTHON_MODULE( mod ) { GeoConstructPyExport::exportGeo<GeoSlab>(mod,"GeoSlab"); }

////////////////////////////////////////////
// Implementation of our geometry module: //
////////////////////////////////////////////

#include "G4Box.hh"
#include "G4Orb.hh"
#include <iostream>

GeoSlab::GeoSlab()
  : GeoConstructBase("G4StdGeometries/GeoSlab")
{
  addParameterDouble("target_depth_cm",0.01);
  addParameterDouble("target_width_cm",100.0);
  addParameterString("material","G4_POLYETHYLENE");
  addParameterDouble("world_extra_margin_cm",0.0);
}

bool GeoSlab::validateParameters()
{
  if ( !(getParameterDouble("world_extra_margin_cm") >= 0.0 ) ) {
    std::cout<< "Error: world_extra_margin_cm must be zero or positive" << std::endl;
    return false;
  }
  if ( !(getParameterDouble("target_depth_cm") >= 0.0 ) ) {
    std::cout<< "Error: target_depth_cm must be positive" << std::endl;
    return false;
  }
  if ( !(getParameterDouble("target_width_cm") >= 0.0 ) ) {
    std::cout<< "Error: target_width_cm must be positive" << std::endl;
    return false;
  }
  return true;
}


G4VPhysicalVolume* GeoSlab::Construct()
{
  //Place target box at positive Z, and identical (for simplicity) recording box at negative Z

  const double world_extra_margin = getParameterDouble("world_extra_margin_cm")*Units::cm;
  const double target_depth = getParameterDouble("target_depth_cm")*Units::cm;
  const double target_width = getParameterDouble("target_width_cm")*Units::cm;
  auto mat = getParameterMaterial("material");
  auto mat_vacuum = getMaterial("Vacuum");

  //World is always slightly larger than contained volumes (so the world can be
  //used as a recording volume in all directions):
  const double world_width = 0.51*target_width + world_extra_margin;
  const double world_depth = 21.0*target_depth + world_extra_margin;
  auto worldvols = place(new G4Box("World",
                                   world_width, world_width, world_depth),
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
