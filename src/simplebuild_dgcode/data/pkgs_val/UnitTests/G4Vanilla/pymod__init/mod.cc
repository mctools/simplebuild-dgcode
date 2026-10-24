
//The simplest possible python module in our framework which links to a G4
//library. Trying to import this can expose an "cannot allocate memory in static
//TLS block" issue in the G4 libs, since G4 TLS and python can be problematic.

#include "sbldpy/sbldpy.hh"
#include "G4Version.hh"
#include <string>

SBLDPY_MODULE( _init )
{
  m.def( "G4Version", []() { return std::string( G4Version ); }, "Geant4 version string from G4Version.hh" );
}
