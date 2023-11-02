#include "Utils/Kinematics.hh"
#include <cassert>

double Utils::beta(double ekin, double mass) {
  if (mass==0.0)
    return 1.0;
  double em = ekin/mass;
  if (em>0.001) {
    //apply relativistic formula directly
    const double gamma = 1.0 + em;
    const double betasq = 1.0 - 1.0 / (gamma*gamma);
    assert( betasq >= 0.0 );
    return std::sqrt(betasq);
  } else {
    //expand relativistic formula to avoid numerical issues
    const double em2 = em*em;
    const double c1 = 23/16.;
    const double c2 = 91/64.;
    const double c3 = 1451/1024.;
    return sqrt(0.5 * em) * ( 2.0 - 1.5 * em + em2 * ( c1 - c2 * em + c3 * em2) );
  }
}
