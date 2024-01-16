#ifndef dgcode_UnitsAndConstants_hh
#define dgcode_UnitsAndConstants_hh

// A few convenience units and constants, designed to be mostly compatible with
// those used in Geant4, most notably meaning millimetres, nanoseconds, and MeV
// being unity (we could have also picked e.g. McStas units, but it is more
// efficient to keep e.g. Griff files in Geant4-compatible units). Most constant
// values (and therefore also values of some derived units) are taken from
// NIST/CODATA 2018, with comments below concerning compatibility with numbers
// used in Geant4 as per Geant4-11.1 (it appears numbers in Geant4 last received
// an update in Geant4-10.7):

namespace Constants {

  constexpr double pi = 3.1415926535897932384626433832795028841971694;
  constexpr double c_light  = 2.99792458e2;
  constexpr double c_squared = c_light * c_light;

  //Atomic unit of charge from NIST/CODATA 2018 (Geant4 also use this value
  //since G4v10.7):
  constexpr double atomic_unit_of_charge  = 1.602176634e-19;

  // Source of Avogadro's constant is NIST/CODATA 2018. This value is the same
  // as in Geant4 since G4v10.7, but has a slight O(1e-7) difference
  // w.r.t. NCrystal-v3.7.1
  // (cf. https://github.com/mctools/ncrystal/issues/149). This is fixed in
  // NCrystal-v3.8.0:
  constexpr double avogadro = 6.02214076e23;

  // NIST/CODATA 2018 definition of h_Planck is 6.62607015e-34*joule*second. The
  // joule*second factor results in a factor of 1e3/atomic_unit_of_charge,
  // hence (Geant4 also use the NIST/CODATA 2018 value since G4v10.7):
  constexpr double h_Planck =  6.62607015e-31 / atomic_unit_of_charge;

  // NIST/CODATA 2018 definition of k_Boltzmann is
  // 1.380649e-23*joule/kelvin. The joule/kelvin factor results in a factor of
  // 1e-6/atomic_unit_of_charge, hence (Geant4 also use the NIST/CODATA 2018
  // value since G4v10.7, however due to limited precision the actual value is
  // O(1e-7) different than the one calculated here):
  constexpr double k_Boltzmann = 1.380649e-29 / atomic_unit_of_charge;

  // NIST/CODATA 2018 definition of neutron_mass_c2 is 939.56542 MeV (this is
  // compatible with NCrystalv3.7.1), but Geant4-v11.1.2 uses 939.56536MeV. As
  // this is less than O(1e-7), we use the NIST/CODATA 2018 number:
  constexpr double  neutron_mass_c2 = 939.56542;

  //We use the NIST/CODATA 2018 value, Geant4-v11.1.2 uses 938.272013, but as
  //this is less than O(1e-7) difference, we use the NIST/CODATA 2018 number:
  constexpr double  proton_mass_c2 = 938.27208816;

  //We use the NIST/CODATA 2018 value, Geant4-v11.1.2 uses 931.494028, but as
  //this is less than O(1e-7) difference, we use the NIST/CODATA 2018 number:
  constexpr double amu_c2 = 931.49410242;
}

namespace Units {

  //Angles (like in all math code, we obviously always keep angles in radians):
  constexpr double deg = 0.0174532925199432957692369076848861271344287189; // = pi/180

  //Length units:
  constexpr double angstrom = 1e-7;
  constexpr double nm       = 1.0e-6;
  constexpr double um       = 1.0e-3;
  constexpr double mm       = 1.0;
  constexpr double cm       = 10.0;
  constexpr double meter    = 1000.0;
  constexpr double m        = meter;

  //Area and volume:
  constexpr double cm2 = 1.0e2;
  constexpr double cm3 = 1.0e3;
  constexpr double m2 = 1.0e6;
  constexpr double m3 = 1.0e9;
  constexpr double barn = 1e-22;

  //Time:
  constexpr double second  = 1.0e9;
  constexpr double ms = 1e6;
  constexpr double millisecond = 1e6;
  constexpr double microsecond = 1e3;
  constexpr double ns = 1.0;
  constexpr double nanosecond = ns;

  //Energy
  constexpr double meV = 1.0e-9;
  constexpr double eV  = 1.0e-6;
  constexpr double keV = 1.0e-3;
  constexpr double MeV = 1.0;
  constexpr double GeV = 1.0e3;
  constexpr double TeV = 1.0e6;
  constexpr double joule = eV / Constants::atomic_unit_of_charge;

  //Magnetic field:
  constexpr double tesla     = 1e-3; // [volt*second/meter^2]

  //Force:
  constexpr double newton = joule * 1e-3;// [J/m]

  //Pressure:
  constexpr double pascal = newton * 1e-6;   // [N/meter^2]
  constexpr double bar = pascal * 1e5; // definition
  constexpr double atmosphere = 101325 * pascal; // definition
  constexpr double atm = atmosphere;

  //Charge:
  constexpr double coulomb = 1.0 / Constants::atomic_unit_of_charge;

  //Mass:
  constexpr double kilogram = joule * second * second / ( meter * meter );
  constexpr double gram = 1.e-3 * kilogram;
  constexpr double g = gram;

  //Temperature:
  constexpr double kelvin = 1.0;

  //Atomic counting:
  constexpr double mole = 1.0;
}

#endif
