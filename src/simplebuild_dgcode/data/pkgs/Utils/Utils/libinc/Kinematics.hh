#ifndef Utils_Kinematics_hh
#define Utils_Kinematics_hh

//Utilities for kinematic calculations.

#include "Units/Units.hh"
#include <cmath>

namespace Utils {

  //Estimate particle beta value from kinetic energy and mass. This function
  //gives numerically stable results for both relativistic and non-relativistic
  //particles.

  double beta(double ekin, double mass);

  //Get velocity rather than beta:
  inline double velocity(double ekin, double mass)
  {
    return Constants::c_light * beta(ekin,mass);
  }

}

#endif
