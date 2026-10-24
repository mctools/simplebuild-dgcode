#include "sbldpy/sbldpy.hh"
#include "Utils/MaxwellDist.hh"

SBLDPY_MODULE( MaxwellDist )
{
  m.def( "maxwellDist", &Utils::maxwellDist, { "a", "x" } );
  m.def( "maxwellDistCommulative", &Utils::maxwellDistCommulative, { "a", "x" } );
  m.def( "maxwellShoot", &Utils::maxwellShoot, { "a", "rand" } );
  m.def( "maxwellParFromMean", &Utils::maxwellParFromMean, { "mean" } );
  m.def( "maxwellParFromPeak", &Utils::maxwellParFromPeak, { "peak" } );
  m.def( "maxwellMean", &Utils::maxwellMean, { "a" } );
  m.def( "maxwellPeak", &Utils::maxwellPeak, { "a" } );
  m.def( "maxwellRMS", &Utils::maxwellRMS, { "a" } );
  m.def( "thermalMaxwellPar", &Utils::thermalMaxwellPar, { "mass", "temperature" } );
  m.def( "shootThermalSpeed", &Utils::shootThermalSpeed, { "mass", "temperature", "rand" } );
  m.def( "shootThermalEKin", &Utils::shootThermalEKin, { "temperature", "rand" } );
  m.def( "shootThermalWavelength", &Utils::shootThermalWavelength, { "mass", "temperature", "rand" } );
  m.def( "thermalNeutronMaxwellPar", &Utils::thermalNeutronMaxwellPar, { "temperature" } );
  m.def( "shootThermalNeutronSpeed", &Utils::shootThermalNeutronSpeed, { "temperature", "rand" } );
  m.def( "shootThermalNeutronEKin", &Utils::shootThermalNeutronEKin, { "temperature", "rand" } );
  m.def( "shootThermalNeutronWavelength", &Utils::shootThermalNeutronWavelength, { "temperature", "rand" } );
  m.def( "thermalEnergyDistPeak", &Utils::thermalEnergyDistPeak, { "temperature" } );
  m.def( "thermalEnergyDistMean", &Utils::thermalEnergyDistMean, { "temperature" } );
  m.def( "thermalEnergyDistMedian", &Utils::thermalEnergyDistMedian, { "temperature" } );
  m.def( "thermalEnergyDist", &Utils::thermalEnergyDist, { "temperature", "energy" } );
  m.def( "thermalEnergyDistCommulative", &Utils::thermalEnergyDistCommulative, { "temperature", "energy" } );
}
