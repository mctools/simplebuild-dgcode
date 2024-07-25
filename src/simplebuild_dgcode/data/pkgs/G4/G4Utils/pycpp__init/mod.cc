#include "G4Utils/Flush.hh"
#include "Core/Python.hh"
#include "G4Version.hh"

namespace pyG4Utils {
  const char * g4version() { return G4Version.c_str(); }
  const char * g4date() { return G4Date.c_str(); }
}

PYTHON_MODULE( mod )
{
  mod.def("flush", &G4Utils::flush, "flushes the G4cout and G4cerr buffers");
  mod.def("G4Version", &pyG4Utils::g4version, "Geant4 version string from G4Version.hh");
  mod.def("G4Date", &pyG4Utils::g4version, "Geant4 date string from G4Version.hh");
}
