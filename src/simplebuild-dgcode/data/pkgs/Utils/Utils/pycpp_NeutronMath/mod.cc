#include "Core/Python.hh"
#include "Utils/NeutronMath.hh"

PYTHON_MODULE( mod )
{
  mod.def("neutronWavelengthToEKin",Utils::neutronWavelengthToEKin);
  mod.def("neutronEKinToWavelength",Utils::neutronEKinToWavelength);
  mod.def("neutron_angstrom_to_meV",Utils::neutron_angstrom_to_meV);
  mod.def("neutron_angstrom_to_eV",Utils::neutron_angstrom_to_eV);
  mod.def("neutron_meV_to_angstrom",Utils::neutron_meV_to_angstrom);
  mod.def("neutron_eV_to_angstrom",Utils:: neutron_eV_to_angstrom);
  mod.def("neutron_angstrom_to_meters_per_second",Utils::neutron_angstrom_to_meters_per_second);
  mod.def("neutron_meters_per_second_to_angstrom",Utils::neutron_meters_per_second_to_angstrom);
}
