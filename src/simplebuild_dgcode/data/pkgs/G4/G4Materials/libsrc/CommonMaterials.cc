#include "G4Materials/CommonMaterials.hh"

#include "Utils/StringSort.hh"
#include "Utils/Format.hh"

#include "Core/FindData.hh"
#include "Units/Units.hh"
#include "Core/String.hh"

#include "G4NistManager.hh"
#include "G4Version.hh"


G4Material* CommonMaterials::getNISTMaterial(const char* name, const char * print_prefix)
{
  static std::map<std::string,G4Material*,Utils::fast_str_cmp> cache;
  std::string sname;
  sname.clear();
  if (Core::starts_with(name,"G4_")) {
    sname += name;
  } else {
    printf("%sERROR: all G4 NIST DB material names should "
           "start with \"G4_\". Problematic request is \"%s\".\n",
           print_prefix,
           name);
    exit(1);
  }
  auto it = cache.find(sname);
  if (it==cache.end()) {
    //A few custom mappings
    if (sname=="G4_Vacuum") { sname="G4_Galactic"; }
    // else if (sname=="G4_CO2") { sname="G4_CARBON_DIOXIDE"; }
    // else if (sname=="G4_CH4") { sname="G4_METHANE"; }
    //else if (sname=="G4_NYLON-12") { return CommonMaterials::getMaterial_Nylon12(); }
    else if (sname=="G4_Gd2O3") { return CommonMaterials::getMaterial_Gd2O3(); }
    G4Material * mat = G4NistManager::Instance()->FindOrBuildMaterial(sname,true);
    if (!mat) {
      printf("%sERROR: did not find material \"%s\" in G4 NIST DB\n",
             print_prefix,name);
      exit(1);
    }
    cache[sname] = mat;
    return mat;
  } else {
    return it->second;
  }
}

G4Element* CommonMaterials::getNISTElement(const char* name, const char * print_prefix)
{
  static std::map<std::string,G4Element*,Utils::fast_str_cmp> cache;
  static std::string sname;
  sname.clear();
  sname += name;
  auto it = cache.find(sname);
  if (it==cache.end()) {
    G4Element * elem = G4NistManager::Instance()->FindOrBuildElement(sname,true);
    if (!elem) {
      printf("%sERROR: did not find element \"%s\" in G4 NIST DB\n",print_prefix,name);
      exit(1);
    }
    cache[sname] = elem;
    assert(elem);
    return elem;
  } else {
    assert(it->second);
    return it->second;
  }
}

G4Isotope * CommonMaterials::getIsotope_B10()
{
  static G4Isotope * iso = 0;
  if (!iso)
    iso = new G4Isotope("B10", 5, 10);
  return iso;
}

G4Isotope * CommonMaterials::getIsotope_B11()
{
  static G4Isotope * iso = 0;
  if (!iso)
    iso = new G4Isotope("B11", 5, 11);
  return iso;
}

G4Element * CommonMaterials::getElement_Boron(double b10_isotope_fraction)
{
  assert(b10_isotope_fraction==-1.0||(b10_isotope_fraction>=0.0 && b10_isotope_fraction<=1.0));
  if (b10_isotope_fraction<0.0) {
    G4Element * e = 0;
    if (!e)
      e = getNISTElement("B");
    assert(e);
    return e;
  }
  //Careful with 64bit vs 80bit precision when using float keys in a map. Thus,
  //round b10_isotope_fraction to 12 decimals and put in an integer:
  int64_t key(b10_isotope_fraction*1e12 + 0.5);
  static std::map<int64_t,G4Element*> cache;
  auto it = cache.find(key);
  if (it!=cache.end())
    return it->second;
  double fb10(key*1e-12);//use this rather than the full precision, to ensure consistency.
  if (b10_isotope_fraction==1.0) fb10 = 1.0;
  else if (b10_isotope_fraction==0.0) fb10 = 0.0;
  std::string name;
  Utils::string_format(name,"Boron_fB10_%.6f", fb10);
  unsigned n_isotopes = (fb10==0.0||fb10==1.0) ? 1 : 2;
  G4Element* elem = new G4Element(name,"B",n_isotopes);
#if G4VERSION_NUMBER >= 1000
  elem->SetNaturalAbundanceFlag(false);
#else
  elem->SetNaturalAbandancesFlag(false);
#endif
  if (fb10 > 0.0) elem->AddIsotope(getIsotope_B10(), fb10);
  if (fb10 < 1.0) elem->AddIsotope(getIsotope_B11(), 1.0-fb10);
  cache[key]=elem;
  return elem;
}

double CommonMaterials::getNaturalB10IsotopeFraction()
{
  static double f = -1.0;
  if (f<0) {
    G4Element* bnat = getElement_Boron();
    assert(bnat);
    auto iv = bnat->GetIsotopeVector();
    for (unsigned i=0;i<iv->size();++i) {
      if ((*iv)[i]->GetN()==10) {
        f = bnat->GetRelativeAbundanceVector()[i];
        break;
      }
    }
    assert(f>0.19&&f<0.21);//around 19.9%
  }
  return f;
}

double CommonMaterials::getNaturalDensity_Boron()
{
  static double d = -1.0;
  if (d<0.0)
    d = getMaterial_Boron(-1,-1)->GetDensity();
  assert(d>2.3*Units::g/Units::cm3&&d<2.4*Units::g/Units::cm3);
  return d;
}

G4Element * CommonMaterials::getElement_Carbon()
{
  static G4Element * c = 0;
  if (!c)
    c = getNISTElement("C");
  assert(c);
  return c;
}


double CommonMaterials::getNaturalDensity_BoronCarbide()
{
  static double d = -1.0;
  if (d<0.0)
    d = getMaterial_BoronCarbide(-1,-1)->GetDensity();
  assert(d>2.4*Units::g/Units::cm3&&d<2.6*Units::g/Units::cm3);
  return d;
}

double CommonMaterials::getDensity_BoronCarbide(double b10_isotope_fraction)
{
  // Calculate density by scaling density from the natural density and b10
  // fraction. To do so, we assume the density of boron carbide scales with:
  //
  //  mB*4+mC
  //
  // Where mB = fB10*mB10+(1-fB10)*mB11
  //
  // Thus when fB10 != fB10Nat, density changes relatively:
  //
  // [(fB10*mB10+(1-fB10)*mB11)*4+mC] / [(fB10Nat*mB10+(1-fB10Nat)*mB11)*4+mC]
  //   = [(fB10*mB10/mB11+(1-fB10))*4+mC/mB11] / [(fB10Nat*mB10/mB11+(1-fB10Nat))*4+mC/mB11]
  assert(b10_isotope_fraction==-1.0||(b10_isotope_fraction>=0.0 && b10_isotope_fraction<=1.0));
  if (b10_isotope_fraction==-1.0)
    return getNaturalDensity_BoronCarbide();
  static double mC_to_mB11 = -1.0;
  static double mB10_to_mB11 = -1.0;
  static double scale = -1.0;
  if (mC_to_mB11<0.0) {
    mB10_to_mB11 = getIsotope_B10()->GetA() / getIsotope_B11()->GetA();
    mC_to_mB11 = getElement_Carbon()->GetA() / getIsotope_B11()->GetA();
    double fB10_natural = getNaturalB10IsotopeFraction();
    scale = getNaturalDensity_BoronCarbide() / ((fB10_natural * mB10_to_mB11 + (1.0-fB10_natural) ) * 4.0 + mC_to_mB11);
  }
  return scale * ((b10_isotope_fraction * mB10_to_mB11 + (1.0-b10_isotope_fraction)) * 4.0 + mC_to_mB11);
}

G4Material * CommonMaterials::getMaterial_Boron(double b10_isotope_fraction,double density)
{
  if (b10_isotope_fraction<0.0&&density<0.0) {
    static G4Material * mat = 0;
    if (!mat)
      mat = getNISTMaterial("G4_B");
    return mat;
  }
  if (b10_isotope_fraction<0.0)
    b10_isotope_fraction = getNaturalB10IsotopeFraction();
  if (density<0.0)
    density = getNaturalDensity_Boron();

  //Careful with 64bit vs 80bit precision when using float keys in a map. Thus,
  //round to a large number of decimals and put in integers:
  int64_t key_fb10(b10_isotope_fraction*1e12 + 0.5);
  int64_t key_density((density/(Units::g/Units::cm3))*1e10 + 0.5);
  std::pair<int64_t,int64_t> key(key_fb10,key_density);
  static std::map<std::pair<int64_t,int64_t>, G4Material*> cache;
  auto it = cache.find(key);
  if (it!=cache.end())
    return it->second;
  //ensure consistency by using only info in rounded numbers:
  double fb10(key_fb10*1e-12);
  double den(key_density*1e-10*(Units::g/Units::cm3));
  std::string name;
  Utils::string_format(name,"Boron_fB10_%.6f_density_%.6fgcm3", fb10, den/(Units::g/Units::cm3));
  G4Material * mat = new G4Material(name, den, 1);
  mat->AddElement(getElement_Boron(fb10), 1);
  cache[key]=mat;
  return mat;
}

G4Material * CommonMaterials::getMaterial_BoronCarbide(double b10_isotope_fraction,double density,double temperature)
{
  if (b10_isotope_fraction<0.0&&density<0.0&&temperature<0.0) {
    static G4Material * mat = 0;
    if (!mat)
      mat = getNISTMaterial("G4_BORON_CARBIDE");
    return mat;
  }

  if (b10_isotope_fraction<0.0)
    b10_isotope_fraction = getNaturalB10IsotopeFraction();
  if (density<0.0)
    density = getDensity_BoronCarbide(b10_isotope_fraction);
  if (temperature<0.0)
    temperature = 273.15;

  //Careful with 64bit vs 80bit precision when using float keys in a map. Thus,
  //round to a large number of decimals and put in integers:
  int64_t key_fb10(b10_isotope_fraction*1e12 + 0.5);
  int64_t key_density((density/(Units::g/Units::cm3))*1e10 + 0.5);
  int64_t key_temp((temperature/Units::kelvin)*1e10 + 0.5);
  std::pair<int64_t,std::pair<int64_t,int64_t> > key(key_fb10,std::make_pair(key_density,key_temp));
  static std::map<std::pair<int64_t,std::pair<int64_t,int64_t> >, G4Material*> cache;
  auto it = cache.find(key);
  if (it!=cache.end())
    return it->second;

  //ensure consistency by using only info in rounded numbers:
  double fb10(key_fb10*1e-12);
  double den(key_density*1e-10*(Units::g/Units::cm3));
  double temp(key_temp*1e-10*Units::kelvin);
  std::string name;
  Utils::string_format(name,"BoronCarbide_fB10_%.6f_density_%.6fgcm3_temp_%.6fK", fb10, den/(Units::g/Units::cm3),temp/Units::kelvin);
  G4Material * mat_natural = getMaterial_BoronCarbide();

  G4Element * elem_B = getElement_Boron(fb10);
  G4Element * elem_C = getElement_Carbon();
  const unsigned n_B(4), n_C(1);//Assuming perfect B4C (wikipedia BoronCarbide for more info)
  const double massfraction_B = double(n_B*elem_B->GetA())/double(n_B*elem_B->GetA()+n_C*elem_C->GetA());
  G4Material * mat = new G4Material(name,
                                    den,
                                    2,
                                    mat_natural->GetState(),
                                    temp,
                                    mat_natural->GetPressure());
  mat->AddElement(elem_B,massfraction_B);
  mat->AddElement(elem_C,1.0-massfraction_B);

  //Copy ionisation data from natural NIST (note, this number is higher than for either of single B or C):
  mat->GetIonisation()->SetMeanExcitationEnergy(mat_natural->GetIonisation()->GetMeanExcitationEnergy());

  cache[key]=mat;

  return mat;
}

G4Material * CommonMaterials::getMaterial_Vacuum()
{
  static G4Material * mat = 0;
  if (!mat)
    mat = getNISTMaterial("G4_Galactic");
  assert(mat);
  return mat;
}

G4Material * CommonMaterials::getMaterial_Air()
{
  static G4Material * mat = 0;
  if (!mat)
    mat = getNISTMaterial("G4_AIR");
  assert(mat);
  return mat;
}

G4Material * CommonMaterials::getMaterial_Nylon12()
{
  static G4Material * mat = 0;
  if (!mat) {
    //Nylon-12 aka Polyamide 12 (used in e.g. PA 2200). Elemental fractions
    //based on numbers in http://arxiv.org/abs/physics/0311110. Density there
    //seems incorrect though, so we take it from
    //http://www.stelray.com/density_val.htm:
    mat = new G4Material("NYLON-12",1.02*Units::gram/Units::cm3,4/*ncomps*/);
    mat->AddElement(getNISTElement("H"),0.11749);
    mat->AddElement(getNISTElement("C"),0.73045);
    mat->AddElement(getNISTElement("N"),0.07098);
    mat->AddElement(getNISTElement("O"),0.08108);
  }
  assert(mat);
  return mat;
}

G4Material * CommonMaterials::getMaterial_Gd2O3()
{
  //NB: In principle this is a polycrystalline material with cubic (sg 206)
  //structure. If we can figure out the information for an .ncmat file we could
  //use NCrystal here as well (but remove this method and let users specify
  //.ncmat file directly as NCrystal material).

  static G4Material * mat = 0;
  if (!mat) {
    //Density (Pradyot Patnaik. Handbook of Inorganic Chemicals. McGraw-Hill, 2002, ISBN 0-07-049439-8 quoted on wikipedia):
    // 7.407 g/cm3 (15 °C)
    // 7.07 g/cm3 (25 °C)
    //Which perhaps looks like variation due to powder packing?
    //
    //A second source simply lists 7.407 as well, so we use that:
    //
    //http://www.reade.com/products/35-oxides-metallic-powders/227-gadolinium-oxide-powder-gd2o3-gadolinium-oxide-digadolinium-monoxide-digadolinium-trioxide-gadolinia-gadolinium-oxide-gd2o3-gadolinium-sesquioxide-gadolinium-trioxide-gadolinium3-oxide-gadoliniumiii-oxide-astmc888-nuclear-grade-cas-12064-62-9

    mat = new G4Material("Gd2O3",7.407*Units::gram/Units::cm3,2);
    mat->AddElement(getNISTElement("Gd"),2);
    mat->AddElement(getNISTElement("O"),3);
    mat->SetChemicalFormula("Gd2O3");
  }
  assert(mat);
  return mat;
}
