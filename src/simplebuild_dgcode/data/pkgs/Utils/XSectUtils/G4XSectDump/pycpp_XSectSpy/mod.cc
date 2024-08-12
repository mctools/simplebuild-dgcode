#include "Core/Python.hh"
#include "XSectSpySteppingAction.hh"
#include "G4Launcher/Launcher.hh"

namespace {
  void spyG4XSect()
  {
    auto launcher = G4Launcher::Launcher::getTheLauncher();
    if (!launcher)
      throw std::runtime_error("XSectSpy ERROR: Launcher not initialised!\n");
    launcher->setUserSteppingAction([]()
    {
      auto launcher2 = G4Launcher::Launcher::getTheLauncher();
      assert(launcher2);
      return new XSectSpySteppingAction(launcher2->getPhysicsList());
    });
  }

  void spyOneG4XSect()
  {
    auto launcher = G4Launcher::Launcher::getTheLauncher();
    if (!launcher)
      throw std::runtime_error("XSectSpy ERROR: Launcher not initialised!\n");
    launcher->setUserSteppingAction([]()
    {
      auto launcher2 = G4Launcher::Launcher::getTheLauncher();
      assert(launcher2);
      return new XSectSpySteppingAction(launcher2->getPhysicsList(),1);
    });
  }
}

PYTHON_MODULE( mod )
{
  mod.def("install",&spyG4XSect);
  mod.def("installForOneFile",&spyOneG4XSect);
  mod.def("lastWrittenFile",&XSectSpySteppingAction::lastWrittenFile);
  mod.def("lastG4MaterialPrinted",&XSectSpySteppingAction::lastG4MaterialPrinted);
}
