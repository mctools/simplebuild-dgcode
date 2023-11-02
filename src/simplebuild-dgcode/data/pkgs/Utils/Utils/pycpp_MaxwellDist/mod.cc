#include "Core/Python.hh"
#include "Utils/MaxwellDist.hh"

PYTHON_MODULE( mod )
{
  mod.def("maxwellDist",Utils::maxwellDist);
  mod.def("maxwellDistCommulative",Utils::maxwellDistCommulative);
  mod.def("maxwellShoot",Utils::maxwellShoot);
  mod.def("maxwellParFromMean",Utils::maxwellParFromMean);
  mod.def("maxwellParFromPeak",Utils::maxwellParFromPeak);
  mod.def("maxwellMean",Utils::maxwellMean);
  mod.def("maxwellPeak",Utils::maxwellPeak);
  mod.def("maxwellRMS",Utils::maxwellRMS);
  mod.def("thermalMaxwellPar",Utils::thermalMaxwellPar);
  mod.def("shootThermalSpeed",Utils::shootThermalSpeed);
  mod.def("shootThermalEKin",Utils::shootThermalEKin);
  mod.def("shootThermalWavelength",Utils::shootThermalWavelength);
  mod.def("thermalNeutronMaxwellPar",Utils::thermalNeutronMaxwellPar);
  mod.def("shootThermalNeutronSpeed",Utils::shootThermalNeutronSpeed);
  mod.def("shootThermalNeutronEKin",Utils::shootThermalNeutronEKin);
  mod.def("shootThermalNeutronWavelength",Utils::shootThermalNeutronWavelength);
  mod.def("thermalEnergyDistPeak",Utils::thermalEnergyDistPeak);
  mod.def("thermalEnergyDistMean",Utils::thermalEnergyDistMean);
  mod.def("thermalEnergyDistMedian",Utils::thermalEnergyDistMedian);
  mod.def("thermalEnergyDist",Utils::thermalEnergyDist);
  mod.def("thermalEnergyDistCommulative",Utils::thermalEnergyDistCommulative);
}
