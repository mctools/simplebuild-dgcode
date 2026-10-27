#include "sbldpy/sbldpy.hh"
#include "XSectSpySteppingAction.hh"
#include "G4Launcher/Launcher.hh"

namespace {
  void spyG4XSect()
  {
    auto launcher = G4Launcher::Launcher::getTheLauncher();
    if (!launcher) {
      printf("XSectSpy ERROR: Launcher not initialised!\n");
      exit(1);
    }
    launcher->setUserSteppingAction(new XSectSpySteppingAction(launcher->getPhysicsList()));
  }

  void spyOneG4XSect()
  {
    auto launcher = G4Launcher::Launcher::getTheLauncher();
    if (!launcher) {
      printf("XSectSpy ERROR: Launcher not initialised!\n");
      exit(1);
    }
    launcher->setUserSteppingAction(new XSectSpySteppingAction(launcher->getPhysicsList(),1));
  }
}

SBLDPY_MODULE( XSectSpy )
{
  m.def( "install", &spyG4XSect );
  m.def( "installForOneFile", &spyOneG4XSect );
  m.def( "lastWrittenFile", []() { return std::string( XSectSpySteppingAction::lastWrittenFile() ); } );
  m.def( "lastG4MaterialPrinted", []() { return std::string( XSectSpySteppingAction::lastG4MaterialPrinted() ); } );
}
