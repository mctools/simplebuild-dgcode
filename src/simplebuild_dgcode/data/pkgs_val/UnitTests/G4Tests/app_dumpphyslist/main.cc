#include "G4RunManager.hh"
#include "G4Box.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4PVPlacement.hh"
#include "QGSP_BERT_HP.hh"
#include "QGSP_BIC_HP.hh"
#include <string>

namespace {
  class DummyDetectorConstruction final : public G4VUserDetectorConstruction {
  public:
    G4VPhysicalVolume* Construct() override
    {
      G4Box* solidWorld = new G4Box("World", 1,1,1);
      G4Material* mat = new G4Material("some_dummy_material", 1, 1, 1);
      G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, mat, "World");
      return new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), logicWorld, "World", 0, false, 0);
    }
  };
}

int main(int argc,char**argv) {
  auto rm = new G4RunManager;
  rm->SetVerboseLevel(99);//high level to get detailed info of physicslist
  rm->SetUserInitialization(new DummyDetectorConstruction);
  if (argc!=2) {
    printf("Please pick physics list among: QGSP_BERT_HP QGSP_BIC_HP\n");
    return 1;
  }
  if (std::string(argv[1])=="QGSP_BERT_HP") rm->SetUserInitialization(new QGSP_BERT_HP);
  else if (std::string(argv[1])=="QGSP_BIC_HP") rm->SetUserInitialization(new QGSP_BIC_HP);
  else {
    printf("Please pick physics list among: QGSP_BERT_HP QGSP_BIC_HP\n");
    return 1;
  }
  rm->Initialize();
  return 0;
}
