#ifndef Utils_MaxwellDist_hh
#define Utils_MaxwellDist_hh

namespace Utils {

  //Evaluate normalised Maxwell distribution for a>0 and x>=0 (0 when x<0):
  //
  // N*x^2*exp(-x^2/(2*a^2)), N=sqrt(2/pi)/a^3
  //
  double maxwellDist(double a, double x);

  //evaluate the commulative maxwell distribution (i.e. the antiderivative with F(-inf)=0 :
  //
  // erf(x/(sqrt(2)*a) - sqrt(2/pi)*x*exp(-x^2/(2*a^2))/a
  double maxwellDistCommulative(double a, double x);

  //evaluate both simultaneously for efficiency:
  void maxwellDistAndCommulative(double a, double x, double& mw, double& mwcommul);

  //Shoot random value according to Maxwell distribution. Input random number must be uniformly distributed in 0..1
  double maxwellShoot(double a, double rand);

  //Various handy methods for calculating a:
  double maxwellParFromMean(double mean);
  double maxwellParFromPeak(double peak);

  //And for extracting mean, peak and variance given a:
  double maxwellMean(double a);
  double maxwellPeak(double a);
  double maxwellRMS(double a);

  //Thermalised particles of mass m have their speed v distributed according to
  //a Maxwell distribution with a=sqrt(kT/m) where k is Boltzmanns constant and
  //T is the temperature.
  double thermalMaxwellPar(double mass, double temperature);//sqrt(kT/m)
  double shootThermalSpeed(double mass, double temperature, double rand);//v from Maxwell dist with a=sqrt(kT/m)
  double shootThermalEKin(double temperature, double rand);//v from above inserted in 0.5*m*v^2 (mass dependency cancels out)
  double shootThermalWavelength(double mass, double temperature, double rand);//v from above inserted in h/(m*v)

  //For convenience, direct methods for neutrons:
  double thermalNeutronMaxwellPar(double temperature);//sqrt(kT/m)
  double shootThermalNeutronSpeed(double temperature, double rand);//v from Maxwell dist with a=sqrt(kT/m)
  double shootThermalNeutronEKin(double temperature, double rand);//v from above inserted in 0.5*m*v^2 (no mass dep, so same as shootThermalEKin)
  double shootThermalNeutronWavelength(double temperature, double rand);//v from above inserted in h/(m*v)

  //Not actually Maxwell Distributions in the mathematical sense, but thermal
  //energy distributions are included here for convenience:
  double thermalEnergyDistPeak(double temperature);//kT/2 (note energy at most probable velocity is kT)
  double thermalEnergyDistMean(double temperature);//3kT/2
  double thermalEnergyDistMedian(double temperature);//kT * 1.1829869...
  double thermalEnergyDist(double temperature, double energy);//2*(kT)^(-3/2)*sqrt(E/pi)*exp(-E/kT)
  double thermalEnergyDistCommulative(double temperature, double energy);//erf(sqrt(E/kT)) - 2*sqrt(E/kT)*exp(-E/kT)/sqrt(pi)
  void thermalEnergyDistAndCommulative(double temperature, double energy, double& d, double& commul);//faster to evaluate both at once

}


#endif
