#ifndef G4Materials_CommonMaterials_hh
#define G4Materials_CommonMaterials_hh

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4Isotope.hh"

// In Geant4, matter is defined in terms of G4Isotopes, G4Elements and G4Materials:
//
//  * G4Isotope such as B10 or B11. Defined by nuclear constitutients (Z,N) and weight per atom (g/mole)
//  * G4Element such as Boron or Aluminium. One element (fixed Z) defined by a relative mix of isotopes, all of same Z.
//  * G4Material: A relative mix of elements and an overall density.
//
// This file provides utilities for construction of commonly used materials.

namespace CommonMaterials {

  //NB: For convenience most functions return non-const pointers since most G4
  //interfaces do not accept const pointers.

  // **********************************************************************
  // ********** NIST Materials and elements (from G4NistManager) **********
  // **********************************************************************

  //Access NIST materials and elements with standard densities and natural isotope abundances (using the G4NistManager):
  //
  //Example names of commonly used NIST materials are:
  //  "G4_STAINLESS-STEEL", "G4_AIR", "G4_Xe", "G4_CARBON_DIOXIDE", "G4_METHANE"
  G4Material * getNISTMaterial(const char* name, const char * print_prefix="");
  G4Element * getNISTElement(const char* name, const char * print_prefix="");

  //In the following methods, -1.0 implies natural fractions and densities and STP conditions (273.15K, 1atm):

  // **********************************************************************
  // ****************** Enriched Boron and Boron Carbide ******************
  // **********************************************************************

  //Pure Boron:
  G4Isotope * getIsotope_B10();
  G4Isotope * getIsotope_B11();
  double getNaturalB10IsotopeFraction();
  double getNaturalDensity_Boron();
  G4Element * getElement_Boron(double b10_isotope_fraction = -1.0);
  G4Material * getMaterial_Boron(double b10_isotope_fraction = -1.0,double density=-1.0);

  //Carbon element:
  G4Element * getElement_Carbon();//obsolete

  //Boron-Carbide:
  double getNaturalDensity_BoronCarbide();
  double getDensity_BoronCarbide(double b10_isotope_fraction = -1.0 );
  G4Material * getMaterial_BoronCarbide(double b10_isotope_fraction = -1.0,double density=-1.0,double temperature=-1.0);

  // **********************************************************************
  // ************************* Special materials **************************
  // **********************************************************************

  G4Material * getMaterial_Vacuum();//Same as "G4_Galactic" from G4NistManager
  G4Material * getMaterial_Air();//Same as "G4_AIR" from G4NistManager
  G4Material * getMaterial_Nylon12();//Custom, a.k.a. PA 2200
  G4Material * getMaterial_Gd2O3();//Custom (still missing crystal structure)

}

#endif
