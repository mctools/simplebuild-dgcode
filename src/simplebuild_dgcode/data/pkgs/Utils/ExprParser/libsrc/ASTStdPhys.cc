#include "ExprParser/ASTStdPhys.hh"
#include "Units/Units.hh"

namespace ExprParser {

  ExprEntityPtr create_standard_unit(const str_type& name)
  {
    if (name.empty())
      return ExprEntityPtr();

    switch (name[0]) {
#     define UNIT(u) { if (name==#u) return create_constant(Units::u); };
    case 'A':
      if (name=="Aa") return create_constant(Units::angstrom);
      break;
    case 'a':
      UNIT(angstrom);
      UNIT(atmosphere);
      break;
    case 'b':
      UNIT(bar);
      if (name=="barn") return create_constant(1e-28 * Units::m2);
      break;
    case 'c':
      UNIT(cm);
      UNIT(cm2);
      UNIT(cm3);
      UNIT(coulomb);
      break;
    case 'd':
      UNIT(deg);
      if (name=="degree") return create_constant(Units::deg);
      break;
    case 'e':
      UNIT(eV);
      break;
    case 'g':
      UNIT(g);
      UNIT(gram);
      break;
    case 'G':
      UNIT(GeV);
      break;
    case 'j':
      UNIT(joule);
      break;
    case 'k':
      UNIT(kelvin);
      UNIT(keV);
      if (name=="kg") return create_constant(1000.0 * Units::gram);
      UNIT(kilogram);
      break;
    case 'm':
      UNIT(m);
      UNIT(m2);
      UNIT(m3);
      UNIT(meter);
      UNIT(meV);
      UNIT(millisecond);
      UNIT(mm);
      UNIT(mole);
      UNIT(ms);
      if (name=="microsecond") return create_constant( 1e-6 * Units::second );
      break;
    case 'M':
      UNIT(MeV);
      break;
    case 'n':
      UNIT(nanosecond);
      UNIT(newton);
      UNIT(nm);
      UNIT(ns);
      break;
    case 'p':
      UNIT(pascal);
      break;
    case 's':
      UNIT(second);
      break;
    case 't':
      UNIT(tesla);
      break;
    case 'T':
      UNIT(TeV);
      break;
    case 'u':
      UNIT(um);
      break;
    }
    return ExprEntityPtr();
  }

  ExprEntityPtr create_standard_constant(const str_type& name)
  {
    if (name.empty())
      return ExprEntityPtr();

    switch (name[0]) {
#     define CONSTANT(u) { if (name==#u) return create_constant(Constants::u); };
    case 'A':
      CONSTANT(avogadro);
      break;
    case 'c':
      CONSTANT(c_light);
      CONSTANT(c_squared);
      break;
    case 'h':
      CONSTANT(h_Planck);
      break;
    case 'k':
      CONSTANT(k_Boltzmann);
      break;
    case 'n':
      CONSTANT(neutron_mass_c2);
      break;
    case 'p':
      CONSTANT(pi);
      CONSTANT(proton_mass_c2);
      break;
    }
    return ExprEntityPtr();
  }

  ExprEntityPtr create_standard_unit_or_constant(const str_type& name)
  {
    auto p = create_standard_unit(name);
    return p ? p : create_standard_constant(name);
  }

}
