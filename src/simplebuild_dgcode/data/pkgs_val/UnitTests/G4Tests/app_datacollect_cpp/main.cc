#include "G4Tests/GeoTest.hh"
#include "Units/Units.hh"
#include "G4Launcher/Launcher.hh"
#include "G4UserSteppingAction.hh"
#include "G4UserEventAction.hh"
#include "G4RunManager.hh"
#include "G4Version.hh"

class SomeSteppingAction : public G4UserSteppingAction {
  void UserSteppingAction(const G4Step*) {
    //Don't print here, too fragile:
    //printf("I am in some other stepping action...\n");
  }
};

class SomeEventAction : public G4UserEventAction {
  void BeginOfEventAction(const G4Event*) {
    printf("I am in some other event action... Event begin\n");
  }
  void EndOfEventAction(const G4Event*) {
    printf("I am in some other event action... Event end\n");
  }
};

void bad_usage()
{
  printf("Bad usage! Please provide the following arguments:\n\n");
  printf("<nevts> output_filename <testhooks> mode\n\n");
  printf("nevts must be a positive integer and testhooks must be 0 or 1\n");
  exit(1);
}
int main(int argc,char**argv)
{
  if (argc!=5)
    bad_usage();

  int nevts = atoi(argv[1]);
  std::string outfile = argv[2];
  int testhooks = atoi(argv[3]);
  std::string mode = argv[4];
  if (nevts<1) bad_usage();
  if (testhooks!=0&&testhooks!=1) bad_usage();

  auto geo = new G4Tests::GeoTest();
  geo->setParameterDouble("boronThickness_micron",200);
  geo->setParameterDouble("worldExtent_meters",1);

  bool vis(false);

  G4Launcher::Launcher launcher;
  launcher.setGeo(geo);
  launcher.setParticleGun("neutron",
                        0.025*Units::eV,
                        G4ThreeVector(0,0,-1.5*geo->getParameterDouble("worldExtent_meters")*Units::meter),
                        G4ThreeVector(0,0,1));
  launcher.setOutput(outfile.c_str(),mode.c_str());
  launcher.setSeed(1119);
  if (vis)
    launcher.setVis();

#if G4VERSION_NUMBER < 1030
  //Two following two lines were appropriate/needed in older Geant4 with the QGSP_BIC_HP
  //physics list. If not interested in Geant4 10.00.p03 support, it is safe to remove
  //the next two lines (and the if-statement). More info at DGSW-305.
  launcher.cmd_postinit("/process/eLoss/StepFunction 0.1 0.001 um");
  launcher.cmd_postinit("/process/eLoss/minKinEnergy 10 eV");
#endif

  if (testhooks)
  {
    launcher.init();
    auto some_stepping_action= new SomeSteppingAction();
    launcher.setUserSteppingAction(some_stepping_action);
    auto some_event_action= new SomeEventAction();
    launcher.setUserEventAction(some_event_action);
  }


  if (vis)
    launcher.startSession();
  else
    launcher.startSimulation(nevts);
  return 0;
}
