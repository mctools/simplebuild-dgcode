
//The simplest possible python module in our framework which links to a G4
//library. Trying to import this can expose an "cannot allocate memory in static
//TLS block" issue in the G4 libs, since G4 TLS and python can be problematic.

#include "Core/Python.hh"
#include "G4Version.hh"

namespace pyG4Utils {
  const char * g4version() { return G4Version.c_str(); }
}

PYTHON_MODULE( mod )
{
  mod.def("G4Version", &pyG4Utils::g4version, "Geant4 version string from G4Version.hh");
}
