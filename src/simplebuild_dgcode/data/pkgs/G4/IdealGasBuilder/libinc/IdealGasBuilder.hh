#ifndef IdealGasBuilder_hh
#define IdealGasBuilder_hh

///////////////////////////////////////////////////////////////////////////////
//
// IdealGasBuilder
//
// Written by Thomas Kittelmann, January 2015.
//
//
// 1.0 Introduction
//
// The IdealGasBuilder is an utility for creating G4Material's corresponding
// to Single or multi component ideal gasses. A gas is described as a Mixture
// of Components (molecules) which are themselves composed of elements.
//
//
// 2.0 Defining gasses
//
// One can define Components and Mixtures through a programmatic
// object-oriented fashion, adding G4Elements into Components and subsequently
// adding the Components into Mixtures. For that approach, refer to the
// IdealGasComponent.hh and IdealGasMixture.hh for the API. Alternatively, and
// often more conveniently, elements, components and mixtures can be described
// by string "formulas".
//
//
// 2.1 Element formulas:
//
//   For elements with natural isotopic composition, simply supply the chemical
//   symbol such as "H", "He", "Li", etc. Behind the scenes, the G4 NIST
//   database is used to provide the G4Elements.
//
//   If custom isotopic specification is desired, specify the isotopes and
//   their fractional appearance in curly-braces after the chemical symbol
//   like this "He{3|0.3,4|0.7}" (helium with 30% He3 and 70% He4). If the sum
//   of specified isotope fractions is <1.0, the remainder will be filled with
//   other naturally occuring isotopes in their usual proportions. For
//   instance "C{14|0.01}" will be carbon with 1% C14, and the remaining 99%
//   shared between the two naturally occuring isotopes (C12 and C13) in their
//   usual proportions, resulting in 97.94% C12 and 1.06% C13.
//
//
// 2.2 Component formulas:
//
//   Component formulas are simply chemical formulas of the molecules in which
//   each element formula is followed by a count of how many times the element
//   appears in a molecule. So a carbon-dioxide component will be described by
//   a formula such as "CO2" in case of natural isotopes, and like
//   "C{12|1.0}O2" in the case where the carbon-dioxide is based on pure
//   carbon-12.
//
//
// 2.3 Mixture formulas:
//
//   Mixture formulas follow the form
//            "frac_1*comp_1+frac_2*comp_2+...frac_n*comp_n"
//   where frac_i is the fractional appearance of the i'th component and
//   comp_i is the component formula of the i'th component.
//
//   As a special case, a single-component ''mixture'' can be described
//   without the fraction, thus allowing both "1.0*comp_1" and "comp_1".
//
//   Fractions are by default interpreted as by-volume fractions (or by-mole
//   fractions which is the same thing for ideal gasses). In order for the
//   fractions to be interpreted as by-mass fractions, one must append
//   "{bymass}" to the entire formula.
//
//   A few examples might clarify this further:
//
//   Example 1 (70%-25%-5% Ar-CO2-CF4 mixture):
//               "0.7*Ar+0.25*Ar+0.05*CF4"
//   Example 2: 70%-25%-5% Ar-CO2-CF4 mixture, by-mass fractions):
//               "0.7*Ar+0.25*Ar+0.05*CF4{bymass}"
//   Example 3: 90%-10% BF3-CO2 mixture with enriched boron to 98% B10:
//               "0.9*B{10|0.98}F3+0.1*CO2"
//   Example 4: Methane:
//               "CH4"
//   Example 5: Helium-3 gas:
//               "He{3|1.0}"
//
//
// 3.0 Creating G4Materials
//
//   After having defined a gas, either by setting up an Component instance
//   (for single-component gasses), a Mixture instance (for multi-component
//   gasses), or by writing down a formula for a gas mixture (see section 2.3
//   above), it can be used to create G4Material instances. All that is needed
//   in addition is to specify two of the three state variables: pressure,
//   temperature & density. The third variable will be automatically
//   calculated, using the ideal gas assumption and Amagat's law.
//
//   For that purpose, there are four material creation functions:
//
//   createMaterialCalcD(...) : Specify temperature and pressure, calculate density.
//   createMaterialCalcP(...) : Specify density and temperature, calculate pressure.
//   createMaterialCalcT(...) : Specify density and pressure, calculate temperature.
//   createMaterial(...) : Alias for createMaterialCalcD
//
//   Those functions exists both as global functions, allowing one to create a
//   material directly from a mixture formula (cf. section 2.3), as well as
//   methods of the Mixture and Component classes, allowing one to create
//   materials corresponding to gasses specified via those instances.
//
//   Note that leaving pressure and temperature arguments at the default value
//   (-1) will result in STP values of 1atm and 273.15K respectively being
//   used.
//
//   If a name is not specified in the calls, a name will automatically be
//   generated, containing some information about both the mixture and the
//   state parameters. Note that no attempt is made to prevent materials with
//   duplicate names from being created.
//
//   The global material creation functions are found in this file, just
//   below. Consult IdealGasComponent.hh and IdealGasMixture.hh for the
//   methods on those classes.
//
//
// 4.0 Ideas for future development (TODO):
//
//     1) We could also allow "D" as shorthand for "H{2:1.0} (and "T" for
//        H{3:1.0}) (obviously isotopic content would not be allowed)
//
//     2) A material like "0.3*CO2+0.7*C{14:0.01}O2" ends up with two carbon
//        elements with different names and isotopic contents. Should be merged
//        and end up as a single element? Both for efficiency, but also to make
//        sure it will never give problems with G4 physics.

#include "IdealGasBuilder/IdealGasComponent.hh"
#include "IdealGasBuilder/IdealGasMixture.hh"

namespace IdealGas {

  G4Material * createMaterialCalcD(const char * formula, double temp = -1, double pressure = -1, const char * name = 0);//Density will be calculated
  G4Material * createMaterialCalcP(const char * formula, double density, double temp = -1, const char * name = 0);//Pressure will be calculated
  G4Material * createMaterialCalcT(const char * formula, double density, double pressure = -1, const char * name = 0);//Temperature will be calculated
  G4Material * createMaterial(const char * formula, double temp = -1, double pressure = -1, const char * name = 0);//Same as createMaterialCalcD

}

#endif
