#include "sbldpy/sbldpy.hh"
#include "G4Utils/Flush.hh"
#include "G4Version.hh"
#include <string>

SBLDPY_MODULE( _init )
{
  m.def( "flush", &G4Utils::flush, "flushes the G4cout and G4cerr buffers" );
  m.def( "G4Version", []() { return std::string( G4Version ); }, "Geant4 version string from G4Version.hh" );
  m.def( "G4Date", []() { return std::string( G4Date ); }, "Geant4 date string from G4Version.hh" );
}
