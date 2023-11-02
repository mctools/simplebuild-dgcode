#include "G4Utils/GenUtils.hh"
#include "Randomize.hh"
#include <cmath>

G4ThreeVector G4Utils::randIsotropicDirection()
{
  //Very fast method (Marsaglia 1972) for generating points uniformly on the
  //unit sphere, costing approximately ~2.54 calls to rand() and 1 call to
  //sqrt().
  double x0,x1,s;
  do {
    x0 = 2.0*(CLHEP::HepRandom::getTheEngine()->flat())-1.0;
    x1 = 2.0*(CLHEP::HepRandom::getTheEngine()->flat())-1.0;
    s = x0*x0 + x1*x1;
  } while (!s||s>=1);
  double t = 2.0*std::sqrt(1-s);
  return G4ThreeVector(x0*t,x1*t,1.0-2.0*s);
}
