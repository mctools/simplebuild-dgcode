#include "sbldpy/sbldpy.hh"
#include "G4DataCollect/G4DataCollect.hh"

SBLDPY_MODULE( _impl )
{
  m.def( "installHooks",
         []( const char* outputFile, const char* mode ) { G4DataCollect::installHooks( outputFile, mode ); },
         "Installs hooks necessary to record the output of the Geant4 simulation.",
         { "outputFile", "mode" } );
  m.def( "finish", &G4DataCollect::finish, "Uninstall hooks and close output file." );
  m.def( "setMetaData", &G4DataCollect::setMetaData, { "key", "value" } );
  m.def( "setUserData", &G4DataCollect::setUserData, { "key", "value" } );
}
