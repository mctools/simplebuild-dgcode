#include "G4RunManager.hh"
#include "G4Box.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4PVPlacement.hh"
#include "QGSP_BERT.hh"

namespace {
  class DummyDetectorConstruction final : public G4VUserDetectorConstruction {
    G4VPhysicalVolume* Construct() override
    {
      G4Box* solidWorld = new G4Box("World", 1,1,1);
      G4Material* mat = new G4Material("some_dummy_material", 1, 1, 1);
      G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, mat, "World");
      return new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), logicWorld, "World", 0, false, 0);
    }
  };
}

void init_dummy_geant4() {
  auto rm = new G4RunManager;
  rm->SetVerboseLevel(0);
  rm->SetUserInitialization(new DummyDetectorConstruction);
  rm->SetUserInitialization(new QGSP_BERT);
  rm->Initialize();
}
