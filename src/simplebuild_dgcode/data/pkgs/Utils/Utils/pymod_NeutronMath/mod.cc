#include "sbldpy/sbldpy.hh"
#include "Utils/NeutronMath.hh"

SBLDPY_MODULE( NeutronMath )
{
  m.def( "neutronWavelengthToEKin", &Utils::neutronWavelengthToEKin, { "l" } );
  m.def( "neutronEKinToWavelength", &Utils::neutronEKinToWavelength, { "e" } );
  m.def( "neutron_angstrom_to_meV", &Utils::neutron_angstrom_to_meV, { "l_aangstrom" } );
  m.def( "neutron_angstrom_to_eV", &Utils::neutron_angstrom_to_eV, { "l_aangstrom" } );
  m.def( "neutron_meV_to_angstrom", &Utils::neutron_meV_to_angstrom, { "e_meV" } );
  m.def( "neutron_eV_to_angstrom", &Utils::neutron_eV_to_angstrom, { "e_eV" } );
  m.def( "neutron_angstrom_to_meters_per_second", &Utils::neutron_angstrom_to_meters_per_second, { "l_aangstrom" } );
  m.def( "neutron_meters_per_second_to_angstrom", &Utils::neutron_meters_per_second_to_angstrom, { "velocity_m_per_s" } );
}
