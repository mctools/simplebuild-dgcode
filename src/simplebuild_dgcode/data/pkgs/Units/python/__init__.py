"""

Python module providing the same constants and unit conversion values as in the
corresponding Units.hh C++ header. Please refer to that header file for
documentation of all values.

"""

# This file was autogenerated from a C++ header file with an md5sum
# checksum: 276029ddc5167e435ae5645fc0a6d0e3

__all__ = ['units','constants']

class _Units:

    deg = 0.017453292519943295
    angstrom = 1e-07
    nm = 1e-06
    um = 0.001
    mm = 1.0
    cm = 10.0
    meter = 1000.0
    m = 1000.0
    cm2 = 100.0
    cm3 = 1000.0
    m2 = 1000000.0
    m3 = 1000000000.0
    barn = 1e-22
    second = 1000000000.0
    ms = 1000000.0
    millisecond = 1000000.0
    ns = 1.0
    nanosecond = 1.0
    meV = 1e-09
    eV = 1e-06
    keV = 0.001
    MeV = 1.0
    GeV = 1000.0
    TeV = 1000000.0
    joule = 6241509074460.763
    tesla = 0.001
    newton = 6241509074.460763
    pascal = 6241.509074460762
    bar = 624150907.4460763
    atmosphere = 632420906.9697367
    atm = 632420906.9697367
    coulomb = 6.241509074460763e+18
    kilogram = 6.241509074460762e+24
    gram = 6.241509074460762e+21
    g = 6.241509074460762e+21
    kelvin = 1.0
    mole = 1.0

    def __setattr__(self,name,value):
        raise AttributeError('Units are read-only ')
    def __delattr__(self,name,value):
        raise AttributeError('Units are read-only ')

class _Constants:

    pi = 3.141592653589793
    c_light = 299.792458
    c_squared = 89875.51787368178
    atomic_unit_of_charge = 1.602176634e-19
    avogadro = 6.02214076e+23
    h_Planck = 4.135667696923859e-12
    k_Boltzmann = 8.617333262145178e-11
    neutron_mass_c2 = 939.56542
    proton_mass_c2 = 938.27208816
    amu_c2 = 931.49410242

    def __setattr__(self,name,value):
        raise AttributeError('Constants are read-only ')
    def __delattr__(self,name,value):
        raise AttributeError('Constants are read-only ')

units = _Units()
constants = _Constants()
