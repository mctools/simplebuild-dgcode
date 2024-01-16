#ifndef G4Utils_GenUtils_hh
#define G4Utils_GenUtils_hh

#include "G4ThreeVector.hh"

namespace G4Utils
{
  //Sample direction on unit-sphere using the standard G4 stream of random
  //numbers:
  G4ThreeVector randIsotropicDirection();
}

#endif
