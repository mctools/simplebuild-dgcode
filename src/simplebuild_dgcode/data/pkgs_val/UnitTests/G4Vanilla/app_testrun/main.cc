#include "G4RunManager.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4PhysListFactory.hh"
#include "G4NistManager.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"

class MyGeo : public G4VUserDetectorConstruction {
public:
  MyGeo() {}
  virtual ~MyGeo() {}
  virtual G4VPhysicalVolume *Construct() {
    G4VSolid * world_shape = new G4Box("World", 1*CLHEP::m, 1*CLHEP::m, 1*CLHEP::m);
    G4Material * mat_air = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR", true);
    G4LogicalVolume * world_logvol = new G4LogicalVolume(world_shape, mat_air, "World");
    G4PVPlacement * world_phys = new G4PVPlacement(0, G4ThreeVector(), world_logvol, "World", 0, false, 0);

    return world_phys;
  }
};

class MyGun : public G4VUserPrimaryGeneratorAction {
public:
  MyGun() : m_gun(new G4ParticleGun(1)) {
    m_gun->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("neutron"));
    m_gun->SetParticleEnergy(5 * CLHEP::MeV);
    m_gun->SetParticlePosition(G4ThreeVector(0.0, 0.0, -1.0 * CLHEP::cm));
    m_gun->SetParticleMomentumDirection(G4ThreeVector(0.0, 0.0, 1.0));
  }

  virtual ~MyGun() {
    delete m_gun;
  }

  void GeneratePrimaries(G4Event *evt) {
    m_gun->GeneratePrimaryVertex(evt);
  }

private:
  G4ParticleGun *m_gun;
};

int main(int, char **) {
  CLHEP::HepRandom::setTheSeed(123456);
  G4RunManager *rm = new G4RunManager;
  rm->SetUserInitialization(new MyGeo);
  rm->SetUserInitialization(G4PhysListFactory().GetReferencePhysList("QGSP_BERT"));
  rm->SetUserAction(new MyGun);
  rm->Initialize();
  rm->BeamOn(10);
  delete rm;
  return 0;
}
