#ifndef G4Materials_NamedMaterialProvider_hh
#define G4Materials_NamedMaterialProvider_hh

// Interface to material creation which makes sure that all materials can be
// defined at configuration time by a simple string.
//
// Most importantly, NCrystal cfg-strings are accepted directly!! So you can
// create crystalline materials, gas mixtures, multiphase-materials, etc. as per
// the NCrystal cfg-string documentation (start at
// https://github.com/mctools/ncrystal/wiki/Using-NCrystal).
//
// Secondly, we have a few convenience aliases:
//
//  "Vacuum" is an alias for the G4_Galactic material.
//  "MAT_Al" is an alias for the NCrystal cfg-string "stdlib::Al_sg225.ncmat"
//  "MAT_Cu" is an alias for the NCrystal cfg-string "stdlib::Cu_sg225.ncmat"
//  "MAT_Ti" is an alias for the NCrystal cfg-string "stdlib::Tu_sg194.ncmat"
//  "MAT_V"  is an alias for the NCrystal cfg-string "stdlib::V_sg229.ncmat"
//
// It is also possible to use the Geant4 NIST manager to create materials, and
// possibly override their temperatur and/or density. Here are a few examples:
//
//   "G4_STAINLESS-STEEL"
//   "G4_AIR"
//   "G4_STAINLESS-STEEL;density_gcm3=8.0;temp_kelvin=100"
//   "G4_STAINLESS-STEEL;scale_density=1.2"
//   "G4_Xe;density_kgm3=1.2""
//   "G4_Gd2O3" <-- a special material added only in dcode.
//
// Note that NCrystal's "gasmix::air[optionalparametershere]" might provide a
// more detailed "air" than "G4_AIR".
//
// Support for B10-enriched B4C is also provided:
//
// "MAT_B4C;b10_enrichment=0.95;density_gcm3=2.4"
// "MAT_B4C;b10_enrichment=0.95" # calculated density based on b10 level.
//
// Finally, the MIX keyword can be used to perform lengthy phase mixtures
// (although NCrystal's native "phases<...>" support might often be better):
//
// "MIX:comp1=[MAT_B4C:b10_enrichment=0.80]:f1=0.8:comp2=[G4_C]
//  :f2=0.14:comp3=[Al_sg225.ncmat]:f3=0.06:allowstatemix=true:density_gcm3=1.36"
//
// The special material MAT_POLYETHYLENE is used to create a G4 material whose
// which works with Geant4's own thermal scattering physics (if enabled in the
// physics list with "+TS"). However, it is anyway recommended to use the
// NCrystal-provided polyethylene instead ("stdlib::Polyethylene_CH2.ncmat").
//
// For backwards compatibility reasons, a few other types of strings are
// currently supported (including "SHIELDING_xxx" and "IdealGasBuilder:xxx"
// materials), but these are not in general recommended.

//   "G4_CO2" is an alias for "G4_CARBON_DIOXIDE" (deprecated)
//   "G4_CH4" is an alias for "G4_METHANE" (deprecated)
//


#include <string>
class G4Material;

namespace NamedMaterialProvider {

  //Access materials:
  G4Material * getMaterial(const std::string&);

  //Normally this will be called by the framework:
  void setPrintPrefix(const char*);
}
#endif
