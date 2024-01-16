#include "Utils/MaxwellDist.hh"
#include "Utils/NewtonRaphson.hh"
#include "Units/Units.hh"
#include <cmath>
#include <cassert>

#define UTILSMW_2DIVSQRTPI  (1.12837916709551256)//2/sqrt(pi)
#define UTILSMW_SQRT_2DIVPI (0.79788456080286541)//sqrt(2/pi)
#define UTILSMW_SQRT_HALF   (0.70710678118654757)//sqrt(1/2) = 1/sqrt(2)
#define UTILSMW_SQRT_PIDIV8 (0.62665706865775006)//sqrt(pi/8)
#define UTILSMW_SQRT_2      (1.4142135623730951)//sqrt(2)
#define UTILSMW_SQRT_8DIVPI (1.5957691216057308)//sqrt(8/pi)
#define UTILSMW_RMSCONST    (0.67343961164285138)//sqrt((3pi-8)/pi)

double Utils::maxwellDist(double a, double x)
{
  assert(a>0);
  if (x<0)
    return 0;
  double y = x/a;
  double y2 = y*y;
  return UTILSMW_SQRT_2DIVPI * y2 * exp( - 0.5 * y2 ) / a;
}

double Utils::maxwellDistCommulative(double a, double x)
{
  assert(a>0);
  if (x<0)
    return 0;
  double y = x/a;
  return erf(UTILSMW_SQRT_HALF*y) - UTILSMW_SQRT_2DIVPI * y * exp( - 0.5 * y * y );
}

void Utils::maxwellDistAndCommulative(double a, double x, double& mw, double& mwcommul)
{
  assert(a>0);
  if (x<0) {
    mw = mwcommul = 0;
    return;
  }
  double y = x/a;
  double y2 = y*y;
  double k = UTILSMW_SQRT_2DIVPI * exp( - 0.5 * y2 ) * y;
  mw =  k * y / a;
  mwcommul = erf(UTILSMW_SQRT_HALF*y) - k;
}

class maxwellShoot_internal_NRFunc : public Utils::NRFunc {
public:
  maxwellShoot_internal_NRFunc(double a, double rand)
    : m_a(a), m_rand(rand)
  {
    assert(a>0);
    assert(rand>=0.0&&rand<=1.0);
  }
  void evaluate(const double& x, double& f_of_x, double& fprime_of_x) const
  {
    if (x<=0) {
      f_of_x = fprime_of_x = 0;
    }
    Utils::maxwellDistAndCommulative(m_a, x, fprime_of_x, f_of_x);
    f_of_x -= m_rand;
    //TODO: cache some constants depending on a and calculate directly here!
  }

private:
  double m_a;
  double m_rand;
};


double Utils::maxwellShoot(double a, double rand)
{
  maxwellShoot_internal_NRFunc func(a,rand);
  return Utils::findNewtonRaphsonRoot(func, 0.0, a * 1.0e15, 1.0e-10);//fixme: max loop parameter?
}

double Utils::maxwellParFromMean(double mean)
{
  return UTILSMW_SQRT_PIDIV8 * mean;
}


double Utils::maxwellParFromPeak(double peak)
{
  return peak * UTILSMW_SQRT_HALF;
}

//And for extracting mean, peak and variance given a:
double Utils::maxwellMean(double a)
{
  return UTILSMW_SQRT_8DIVPI * a;
}

double Utils::maxwellPeak(double a)
{
  return UTILSMW_SQRT_2 * a;
}

double Utils::maxwellRMS(double a)
{
  return UTILSMW_RMSCONST * a;
}

double Utils::thermalMaxwellPar(double mass, double temperature)
{
  return sqrt(Constants::k_Boltzmann*temperature/mass);
}

double Utils::shootThermalSpeed(double mass, double temperature, double rand)
{
  return maxwellShoot(thermalMaxwellPar(mass,temperature), rand);
}

double Utils::shootThermalEKin(double temperature, double rand)
{
  //mass dependency cancels out, so using mass=1.0 here:
  double v = shootThermalSpeed(1.0,temperature, rand);
  //and here:
  return 0.5*v*v;
}

double Utils::shootThermalWavelength(double mass, double temperature, double rand)
{
  return Constants::h_Planck / (mass*shootThermalSpeed(mass,temperature, rand));
}

double Utils::thermalNeutronMaxwellPar(double temperature)
{
  return thermalMaxwellPar(Constants::neutron_mass_c2/Constants::c_squared,temperature);
}

double Utils::shootThermalNeutronSpeed(double temperature, double rand)
{
  return shootThermalSpeed(Constants::neutron_mass_c2/Constants::c_squared,temperature,rand);
}

double Utils::shootThermalNeutronEKin(double temperature, double rand)
{
  return shootThermalEKin(temperature,rand);
}

double Utils::shootThermalNeutronWavelength(double temperature, double rand)
{
  return shootThermalWavelength(Constants::neutron_mass_c2/Constants::c_squared,temperature,rand);
}

double Utils::thermalEnergyDistPeak(double temperature)
{
  return 0.5*Constants::k_Boltzmann*temperature;
}

double Utils::thermalEnergyDistMean(double temperature)
{
  return 1.5*Constants::k_Boltzmann*temperature;
}

double Utils::thermalEnergyDistMedian(double temperature)
{
  return 1.1829869421876691330695813*Constants::k_Boltzmann*temperature;
}

double Utils::thermalEnergyDist(double temperature, double energy)
{
  assert(temperature>0);
  if (energy<0)
    return 0;
  double b = 1.0 / ( Constants::k_Boltzmann*temperature );
  double x = energy * b;
  return UTILSMW_2DIVSQRTPI*b*sqrt(x)*exp(-x);
}

double Utils::thermalEnergyDistCommulative(double temperature, double energy)
{
  assert(temperature>0);
  if (energy<0)
    return 0;
  double x = energy / ( Constants::k_Boltzmann*temperature );
  double sx = sqrt(x);
  return erf(sx) - UTILSMW_2DIVSQRTPI * sx * exp(-x);
}

void Utils::thermalEnergyDistAndCommulative(double temperature, double energy, double& d, double& commul)
{
  assert(temperature>0);
  if (energy<0) {
    d = commul = 0;
    return;
  }
  double b = 1.0 / ( Constants::k_Boltzmann*temperature );
  double x = energy * b;
  double sx = sqrt(x);
  double ex = exp(-x);
  double k = UTILSMW_2DIVSQRTPI*sx*ex;
  d = k*b;
  commul = erf(sx) - k;
}
