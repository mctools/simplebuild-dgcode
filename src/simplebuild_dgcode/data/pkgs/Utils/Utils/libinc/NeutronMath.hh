#ifndef Utils_NeutronMath_hh
#define Utils_NeutronMath_hh

#include "Units/Units.hh"
#include <limits>//for infinity
#include <cmath>

//TODO consistent naming of functions

namespace Utils {

  inline double neutronWavelengthToEKin(double l) {
    if (l==0)
      return std::numeric_limits<double>::infinity();
    return 0.5 * Constants::h_Planck * Constants::h_Planck * Constants::c_squared / (l*l*Constants::neutron_mass_c2);
  }

  inline double neutronEKinToWavelength(double e) {
    if (e==0)
      return std::numeric_limits<double>::infinity();
    return Constants::h_Planck*Constants::c_light*sqrt(0.5/(Constants::neutron_mass_c2*e));
  }

  inline double neutron_angstrom_to_meV(double l_aangstrom) {
    if (l_aangstrom==0)
      return std::numeric_limits<double>::infinity();
    double l = l_aangstrom*Units::angstrom;
    double res =  0.5 * Constants::h_Planck * Constants::h_Planck * Constants::c_squared / (l*l*Constants::neutron_mass_c2);
    return res / Units::meV;
  }

  inline double neutron_angstrom_to_eV(double l_aangstrom) {
    return neutron_angstrom_to_meV(l_aangstrom)*0.001;
  }

  inline double neutron_meV_to_angstrom(double e_meV) {
    if (e_meV==0)
      return std::numeric_limits<double>::infinity();
    double e = e_meV*Units::meV;
    double l = Constants::h_Planck * Constants::c_light * sqrt(0.5/(Constants::neutron_mass_c2*e));
    return l/Units::angstrom;
  }

  inline double neutron_eV_to_angstrom(double e_eV) {
    return neutron_meV_to_angstrom(1000.0*e_eV);
  }

  inline double neutron_angstrom_to_meters_per_second(double l_aangstrom) {
    if (l_aangstrom==0)
      return std::numeric_limits<double>::infinity();
    return Constants::h_Planck * Units::second*Constants::c_squared / (Units::m*Units::angstrom * Constants::neutron_mass_c2 * l_aangstrom);
  }

  inline double neutron_meters_per_second_to_angstrom(double velocity_m_per_s) {
    if (velocity_m_per_s==0)
      return std::numeric_limits<double>::infinity();
    constexpr double kkk = Constants::h_Planck * Units::second*Constants::c_squared / (Units::m*Units::angstrom * Constants::neutron_mass_c2);
    return kkk / velocity_m_per_s;
  }

}

#endif
