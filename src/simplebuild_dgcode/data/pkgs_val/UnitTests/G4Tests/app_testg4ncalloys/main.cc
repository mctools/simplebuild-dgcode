#include "G4Materials/G4NCUtils.hh"
#include "G4Material.hh"
#include <iostream>

//NOTICE: Apart from the mat_sample = ... line, this whole thing here was copied from the g4ncrystal example!!

#include "G4NCrystalRel/G4NCrystal.hh"
#include "G4RunManager.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysListFactory.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4PVPlacement.hh"
#include "G4VSensitiveDetector.hh"
#include "G4SDManager.hh"
#include "G4StepPoint.hh"

class MySD final : public G4VSensitiveDetector {
  //////////////////////////////////////////////////////////////////////////////
  // Sensitive detector for monitoring neutron hits in the spherical detector
  // volume and printing out the "detected" scattering angle.
  //////////////////////////////////////////////////////////////////////////////
public:
  MySD() : G4VSensitiveDetector("MySD") {}
  virtual ~MySD(){}

  G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override
  {
    if (step->GetPreStepPoint()->GetStepStatus() != fGeomBoundary)
      return true;//must have just entered the volume
    if (step->GetTrack()->GetDynamicParticle()->GetPDGcode()!=2112)
      return true;//must be neutron
    G4ThreeVector pos = step->GetPreStepPoint()->GetPosition();
    double r = sqrt(pos.x()*pos.x()+pos.y()*pos.y());
    if (pos.z()>0&&r<0.001*mm)
      return true;//No scattering took place
    printf("Hit detected at theta = %.0f deg!\n",atan2(r,pos.z())*180/M_PI);
    return true;
  }
};

class MyGeo final : public G4VUserDetectorConstruction {
  //////////////////////////////////////////////////////////////////////////////
  // Constructs an r=1*mm spherical sample of polycrystalline Aluminium inside an
  // r=100*cm spherical vacuum inside a 1*mm thick spherical counting volume,
  // inside a 110*cm world box. The sample is small enough for multiple neutron
  // scattering events to be negligible and the detector is far enough from the
  // sample to make sample size effects on the angular measurement equally
  // negligible.
  //////////////////////////////////////////////////////////////////////////////
public:
  MyGeo(){}
  virtual ~MyGeo(){}
  G4VPhysicalVolume* Construct() override
  {
    G4Material * mat_vacuum = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic",true);
    //G4Material * mat_sample = G4NCrystalRel::createMaterial("Al_sg225.ncmat;bkgd=0");
    //G4Material * mat_sample = G4NCrystalRel::createMaterial("MgO_sg225_Periclase.ncmat;bkgd=0");
    G4Material * mat_sample = NCG4Utils::createPCAlloy("Alloy_Al75_MgO25", NCG4Utils::AlloyList({{"Al_sg225.ncmat;bkgd=0",0.75},{"MgO_sg225_Periclase.ncmat;bkgd=0",0.25}}));
    std::cout<<*mat_sample<<std::endl;
    G4LogicalVolume * world_log = new G4LogicalVolume(new G4Box("world",110*cm,110*cm,110*cm),
                                                      mat_vacuum,"world",0,0,0);
    G4PVPlacement * world_phys = new G4PVPlacement(0,G4ThreeVector(),world_log,"world",0,false,0);
    G4LogicalVolume * det_log = new G4LogicalVolume(new G4Sphere("detector",0,100.1*cm,0,2*M_PI,0,M_PI),
                                                      mat_vacuum,"detector",0,0,0);
    new G4PVPlacement(0,G4ThreeVector(),det_log,"detector",world_log,false,0);
    G4LogicalVolume * vacuum_log = new G4LogicalVolume(new G4Sphere("vacuum",0,100.0*cm,0,2*M_PI,0,M_PI),
                                                       mat_vacuum,"vacuum",0,0,0);
    new G4PVPlacement(0,G4ThreeVector(),vacuum_log,"vacuum",det_log,false,0);
    G4LogicalVolume * sample_log = new G4LogicalVolume(new G4Sphere("sample",0,0.1*cm,0,2*M_PI,0,M_PI),
                                                       mat_sample,"sample",0,0,0);
    new G4PVPlacement(0,G4ThreeVector(),sample_log,"sample",vacuum_log,false,0);
    MySD * sd = new MySD();
    G4SDManager::GetSDMpointer()->AddNewDetector( sd );
    det_log->SetSensitiveDetector(sd);
    return world_phys;
  }
};

class MyGun final : public G4VUserPrimaryGeneratorAction {
  //////////////////////////////////////////////////////////////////////////////
  // Setup monochromatic source of neutrons, hitting the sample with initial direction (0,0,1)
  //////////////////////////////////////////////////////////////////////////////
public:

  MyGun(double neutron_wavelength) : m_particleGun(new G4ParticleGun(1))
  {
    m_particleGun->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("neutron"));
    m_particleGun->SetParticleEnergy(neutronWavelengthToEKin(neutron_wavelength));
    m_particleGun->SetParticlePosition(G4ThreeVector(0.0, 0.0, -1.0*cm));
    m_particleGun->SetParticleMomentumDirection(G4ThreeVector(0.0, 0.0, 1.0));
  }

  virtual ~MyGun()
  {
    delete m_particleGun;
  }

  void GeneratePrimaries(G4Event* evt) override
  {
    m_particleGun->GeneratePrimaryVertex(evt);
  }

  double neutronWavelengthToEKin(double l) {
    return 0.5 * CLHEP::h_Planck * CLHEP::h_Planck * CLHEP::c_squared / (l*l*CLHEP::neutron_mass_c2);
  }

private:
  G4ParticleGun* m_particleGun;
};


int main(int,char**) {
  //Set seed:
  CLHEP::HepRandom::setTheSeed(123);

  //G4 Run manager:
  G4RunManager* runManager = new G4RunManager;

  //Setup geometry and physics-list:
  runManager->SetUserInitialization(new MyGeo);
  runManager->SetUserInitialization(G4PhysListFactory().GetReferencePhysList("QGSP_BIC_HP"));

  //Setup monochromatic source of 4.0Aa neutrons. Note that at 4.0 Aangstrom,
  //more than 90% of scattering events in polycrystalline aluminium are coherent
  //with peaks only at theta = 118 deg and theta = 162 deg:
  const double neutron_wavelength = 4.1*angstrom;
  runManager->SetUserAction(new MyGun(neutron_wavelength));

  //Initialize g4 run manager:
  runManager->Initialize();

  //Install G4NCrystal:
  G4NCrystalRel::installOnDemand();

  //Run 1000 events (should give us ~20 scattering events on average for 4.0 aangstrom):
  runManager->BeamOn(1000);

  //Cleanup:
  delete runManager;

  G4NCrystalRel::Manager::cleanup();//delete manager singleton, unref cached ncrystal objects (for valgrind).

  return 0;
}
