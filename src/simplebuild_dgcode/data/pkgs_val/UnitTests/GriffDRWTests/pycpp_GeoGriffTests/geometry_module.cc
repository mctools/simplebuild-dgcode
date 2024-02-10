#include "G4Interfaces/GeoConstructPyExport.hh"
#include "G4Box.hh"
#include "G4Orb.hh"

class GeoGriffTests final : public G4Interfaces::GeoConstructBase
{
public:
  GeoGriffTests() : GeoConstructBase("G4GeoGriffTests/GeoGriffTests") {}
  virtual ~GeoGriffTests(){}
  G4VPhysicalVolume* Construct() override;
};

PYTHON_MODULE( mod ) { GeoConstructPyExport::exportGeo<GeoGriffTests>(mod, "GeoGriffTests"); }

G4VPhysicalVolume* GeoGriffTests::Construct()
{
  auto mat = getMaterial("Vacuum");

  double dw = 10*Units::m;
  auto world = place(new G4Box("World", dw,dw,dw),mat,0.,0.,0.,0,INVISIBLE);

  //Vols 1 lvl down:
  double d1 = 3*Units::m;
  auto vola = place(new G4Box("TestVolA", d1,d1,d1),mat,-1.5*d1,0.1*d1,0.,world.logvol,RED);
  auto logvolb = new G4LogicalVolume(new G4Orb("ShapeB", d1),mat,"LogVolB");
  auto volb = place(logvolb,+1.5*d1,0.,0.1*d1,world.logvol,YELLOW);
  place(new G4Box("TestVolC", d1,d1,d1),mat,0,-1*d1,-2.2*d1,world.logvol,BLUE);

  //Vols 2 lvls down:
  double d2 = 1*Units::m;
  auto volaa = place(new G4Box("TestVolAA", d2,d2,d2),mat,0.5*d2,0.9*d2,d2,vola.logvol,GREEN);
  place(new G4Orb("TestVolAB", d2),mat,0,+1*d2,-1.9*d2,vola.logvol,BLUE);
  place(new G4Box("TestVolAC", d2,d2,d2),mat,+1.5*d2,-1.9*d2,0.,vola.logvol,YELLOW);
  auto volba = place(new G4Box("TestVolBA", d2,d2,d2),mat,0,0.2*d2,0.,volb.logvol,RED);

  //Vols 3 lvls down:
  double d3 = 0.5*Units::m;
  place(new G4Orb("TestVolAAA", d3),mat,0,0.5*d3,-0.2*d3,volaa.logvol,BLUE);
  place(new G4Box("TestVolBAA", d3,d3,d3*0.5),mat,0,-0.2*d3,0.1*d3,volba.logvol,YELLOW);

  return world.physvol;
}
