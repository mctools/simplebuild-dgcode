#include "G4Launcher/Launcher.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "Units/Units.hh"
#include "G4NistManager.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4VisAttributes.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"

namespace {
  class DummyGeo final : public G4Interfaces::GeoConstructBase {
  public:

    DummyGeo() : GeoConstructBase("DummyGeo")
    {
      addParameterDouble("boxThickness",1*Units::um,0.01*Units::um,300*Units::cm);
    }

    G4VPhysicalVolume* Construct() override
    {
      G4Material* mat_galactic = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic",true);
      G4Material* mat_boroncarbide = G4NistManager::Instance()->FindOrBuildMaterial("G4_BORON_CARBIDE",true);
      G4Box* solidWorld = new G4Box("World", 10*Units::meter,10*Units::meter,10*Units::meter);
      G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, mat_galactic, "World");
      auto pv_world = new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), logicWorld, "World", 0, false, 0);
      G4Box* solidBox = new G4Box("SldSomeBox", 5*Units::meter,5*Units::meter,getParameterDouble("boxThickness")*0.5);
      auto logicBox= new G4LogicalVolume(solidBox, mat_boroncarbide, "LVSomeBox");
      new G4PVPlacement(0, G4ThreeVector(0.0,0.0,0.0),"PVSomeBox", logicBox, pv_world, false, 0, true);

      logicWorld -> SetVisAttributes (G4VisAttributes::GetInvisible());

      return pv_world;
    }
  };
}

int main(int,char**) {

  DummyGeo * geo = new DummyGeo();
  geo->setParameterDouble("boxThickness",20*Units::cm);

  G4Launcher::Launcher launcher;
  launcher.setGeo(geo);
  launcher.setParticleGun(2112, 0.025*Units::eV, G4ThreeVector(0,0,-5*Units::meter), G4ThreeVector(0,0,1));
  launcher.setOutput("myoutput.griff","REDUCED");
  launcher.setVis();
  if (false) {
    launcher.initVis();
    launcher.cmd("/vis/scene/add/axes 0 0 0 500 mm");
  }
  launcher.startSession();
}
