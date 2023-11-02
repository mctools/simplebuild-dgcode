#ifndef ExprParser_ASTStdPhys_hh
#define ExprParser_ASTStdPhys_hh

#include "ExprParser/ASTNode.hh"

namespace ExprParser {

  //Helper functions which might be used in the createValue method of custom
  //ASTBuilders to provide physics units and constants, based on the
  //SystemOfUnits/PhysicalConstants from CLHEP & Geant4. Note that this just
  //returns floating point constants, thus not helping to ensure in any way that
  //parsed expressions are consistent unit-wise. E.g. one might happily write
  //and evaluate something non-sensical as "sin(MeV)".

  ExprEntityPtr create_standard_unit(const str_type& name);
  ExprEntityPtr create_standard_constant(const str_type& name);
  ExprEntityPtr create_standard_unit_or_constant(const str_type& name);

}

#endif
