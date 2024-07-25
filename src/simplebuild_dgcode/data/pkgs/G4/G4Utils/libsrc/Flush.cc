#include "G4Utils/Flush.hh"
#include "G4ios.hh"

void G4Utils::flush()
{
  G4cout << std::flush;
  G4cerr << std::flush;
  // G4coutbuf.sync();
  // G4cerrbuf.sync();
}
