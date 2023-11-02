#include "Core/Python.hh"
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

PYTHON_MODULE( mod )
{
  mod.def("install",&spyG4XSect);
  mod.def("installForOneFile",&spyOneG4XSect);
  mod.def("lastWrittenFile",&XSectSpySteppingAction::lastWrittenFile);
  mod.def("lastG4MaterialPrinted",&XSectSpySteppingAction::lastG4MaterialPrinted);
}
