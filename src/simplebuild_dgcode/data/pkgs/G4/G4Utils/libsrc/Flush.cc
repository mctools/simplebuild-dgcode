#include "G4Utils/Flush.hh"
#include "G4strstreambuf.hh"

void G4Utils::flush()
{
  G4coutbuf.sync();
  G4cerrbuf.sync();
}
