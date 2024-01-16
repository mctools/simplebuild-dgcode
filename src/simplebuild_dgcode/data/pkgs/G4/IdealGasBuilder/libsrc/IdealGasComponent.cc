//See IdealGasBuilder.hh for general information.
//Author: Thomas Kittelmann, January 2015.

#include "IdealGasBuilder/IdealGasComponent.hh"
#include "IdealGasBuilder/IdealGasMixture.hh"
#include "G4NistManager.hh"
#include "G4Element.hh"
#include "G4Version.hh"
#include <cassert>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>
#include <map>

IdealGas::Component::Component()
  : m_mass(-1), m_unlocked(true)
{
}

IdealGas::Component::Component(const char * formula)
  : m_mass(-1), m_unlocked(true)
{
  addElements(formula);
  lock();
}

IdealGas::Component::~Component()
{
  //don't do anything
}

void IdealGas::Component::lock() const
{
  m_unlocked = false;
}

void IdealGas::Component::addElement(G4Element* el, double count)
{
  if (!m_unlocked)
    throw std::runtime_error("IdealGas::Component Can't add elements to a locked component");
  if (!el)
    throw std::runtime_error("IdealGas::Component::addElement NULL element passed");
  assert(el);
  m_mass = -1;
  if (count<=0)
    throw std::runtime_error("IdealGas::Component::addElement count must be a positive number");
  m_elements.push_back(std::make_pair(el,count));
  m_name += el->GetName();
  if (count!=1.0) {
    char buf[64];
    int n = snprintf(buf, 64, "%g",count);
    if (n>63)
      throw std::runtime_error("IdealGas::Component::addElement buffer error");
    m_name.append(buf,n);
  }
}

void IdealGas::Component::addElement(const char* formula, double count)
{
  addElement(getElementFromFormula(formula),count);
}

G4Element * IdealGas::getElementFromFormula(const char* formula)
{
  static std::map<std::string,G4Element*> cache;
  auto it = cache.find(formula);
  if (it!=cache.end())
    return it->second;

  assert(formula);
  std::string element_name(formula);
  std::string custom_isotope_info;
  const char * c = formula;
  static const std::string err = "IdealGas::Component::getElementFromFormula ";
  while (*c) {
    if (*c=='{') {
      element_name.resize(c-formula);
      custom_isotope_info += c+1;
      if (custom_isotope_info.size()<4||*custom_isotope_info.rbegin()!='}')
        throw std::runtime_error(err+"invalid isotope formula");
      custom_isotope_info.resize(custom_isotope_info.size()-1);
      break;
    }
    ++c;
  }
  if (element_name.empty())
    throw std::runtime_error(err+"invalid formula");

  const bool buildisotopes = true;
  G4Element * natural_element = G4NistManager::Instance()->FindOrBuildElement(element_name,buildisotopes);
  if (!natural_element)
    throw std::runtime_error(err+"invalid element name: \""+element_name+"\"");

  //A bit annoying that G4NistManager does not honour the buildisotopes flag if
  //the same element was previously requested with a different flag value. For
  //now, we just detect this and throw an error:

#if G4VERSION_NUMBER >= 1000
  const bool natab(natural_element->GetNaturalAbundanceFlag());
#else
  const bool natab(natural_element->GetNaturalAbandancesFlag());
#endif
  if (!natab)
    throw std::runtime_error(err+"G4 NIST element without natural abundances: \""+element_name
                             +"\" (perhaps it was previously built with buildisotopes=false?)");

  G4Element * elem(0);
  std::map<int,double> isomap;
  if (!custom_isotope_info.empty()) {
    //Must decode isotope info and fill the isomap:
    std::string iso_n;
    std::string iso_f;
    const char * ccp(&custom_isotope_info[0]);
    const char * ccpEP1(ccp+custom_isotope_info.size() + 1);
    std::string erriso = err+"invalid isotope specification";
    bool mode_n(true);
    for (;ccp!=ccpEP1;++ccp) {
      if (ccp+1==ccpEP1 || *ccp == ',') {
        //Flush isotope
        if (iso_n.empty()||iso_f.empty())
          throw std::runtime_error(erriso);
        char * sEnd;
        double iso_frac = strtod(iso_f.c_str(),&sEnd);
        if (sEnd!=(&*iso_f.end())||iso_frac<0.0||iso_frac>1.0)
            throw std::runtime_error(erriso);
        long iso_l = strtol(iso_n.c_str(),&sEnd,10);
        if (sEnd!=(&*iso_n.end())||iso_l<1||iso_l>1000)
          throw std::runtime_error(erriso);
        int iso_N = static_cast<int>(iso_l);
        if (isomap.find(iso_N)!=isomap.end())
          throw std::runtime_error(erriso);
        isomap[iso_N] = iso_frac;
        iso_n.clear();
        iso_f.clear();
        mode_n=true;
        continue;
      }
      char cc = *ccp;
      if (cc=='|') {
        if ( !mode_n || iso_n.empty() || !iso_f.empty() )
          throw std::runtime_error(erriso);
        mode_n = false;
      } else {
        bool part_uint(cc>='0'&&cc<='9');
        if (mode_n) {
          if (!part_uint)
            throw std::runtime_error(erriso);
          iso_n += cc;
        } else {
          bool part_float(part_uint||cc=='.'||cc=='e'||cc=='E'||cc=='-'||cc=='+');
          if (!part_float)
            throw std::runtime_error(erriso);
          iso_f += cc;
        }
      }
    }
  }

  if (isomap.empty()) {
    elem = natural_element;
  } else {
    //Custom isotopic content => we have to construct our own element!
    double totalfracs = 0.0;
    for (auto itiso=isomap.begin();itiso!=isomap.end();++itiso) {
      assert(itiso->second>=0.0&&itiso->second<=1.0);
      totalfracs += itiso->second;
    }
    if (totalfracs > 1.0 + 1.0e-6)
      throw std::runtime_error(err+"isotope fractions sum to more than 1.0");
    if (1.0 - totalfracs>1.0e-6) {
      //Must attempt to augment the isomap with other naturally occuring isotopes, and then recalculate totalfracs:
      unsigned natiso_N = (unsigned)natural_element->GetNumberOfIsotopes();
      double * natiso_frac = natural_element->GetRelativeAbundanceVector();
      assert(natiso_N&&natiso_frac);
      const std::vector<G4Isotope*>* natisos = natural_element->GetIsotopeVector();


      double natfrac_other(0.0);
      for (unsigned i=0;i<natiso_N;++i) {
        G4Isotope* natiso = natisos->at(i);
        if (isomap.find(natiso->GetN())==isomap.end())
          natfrac_other += natiso_frac[i];
      }
      if (natfrac_other<=0.0)
        throw std::runtime_error(err+" supplied isotope fractions sum is < 1.0 and there are no other natural isotopes");
      double scale = (1.0 - totalfracs)/natfrac_other;
      for (unsigned i=0;i<natiso_N;++i) {
        G4Isotope* natiso = natisos->at(i);
        if (isomap.find(natiso->GetN())==isomap.end()) {
          isomap[natiso->GetN()] = natiso_frac[i] * scale;
        }
      }
      totalfracs = 0.0;
      for (auto itiso=isomap.begin();itiso!=isomap.end();++itiso) {
        totalfracs += itiso->second;
      }
    }

    int Z = static_cast<int>(0.5 + natural_element->GetZ());
    if (fabs(natural_element->GetZ() - Z) > 1.0e-6)
      throw std::runtime_error(err+"element has non-integer Z!");

    elem = new G4Element(formula,natural_element->GetSymbol(),isomap.size());
    const double unit(CLHEP::g/(CLHEP::mole*CLHEP::amu_c2));
    for (auto itiso=isomap.begin();itiso!=isomap.end();++itiso) {
      std::stringstream os;
      os << natural_element->GetSymbol() << itiso->first;
      double mass=G4NistManager::Instance()->GetAtomicMass(Z,itiso->first)*unit;
      if (mass<=0.0)
        throw std::runtime_error(err+" unknown isotope of "+std::string(natural_element->GetSymbol())
                                 +": "+std::to_string((long long int)itiso->first));
      G4Isotope * iso = new G4Isotope(os.str().c_str(), Z, itiso->first,
                                      mass);
      double relabundance = itiso->second;
      elem->AddIsotope(iso,relabundance/totalfracs);
    }
#if G4VERSION_NUMBER >= 1000
    elem->SetNaturalAbundanceFlag(false);
#else
    elem->SetNaturalAbandancesFlag(false);
#endif
  }

  assert(elem);
  cache[formula] = elem;
  return elem;
}

void IdealGas::Component::addElements(const char * formula)
{
  std::string f(formula);
  std::string symbol;
  std::string isotope_info;
  std::string count;
  bool isotope_section(false);
  const char * err = "IdealGas::Component::addElements error in formula";
  for (unsigned i=0;i<=f.size();++i) {
    char c = i<f.size() ? f[i] : '#';
    bool maybe_digit = ( (c>='0'&&c<='9') || c == '.' || c == 'e' || c=='E' || c=='+' || c=='-' );
    if (isotope_section) {
      if (c=='}') {
        isotope_section=false;
      } else {
        if (c=='{'||i==f.size())
          throw std::runtime_error(std::string(err)+": Incomplete isotope section");
        //Only numbers (0-9eE+-.) and : and , allowed inside isotope braces:
        if (!(maybe_digit||c=='|'||c==','))
          throw std::runtime_error(std::string(err)+": Forbidden character in isotope section");
        isotope_info += c;
      }
      continue;
    } else if (c=='{') {
      if ( symbol.empty() || !isotope_info.empty() || !count.empty() )
        throw std::runtime_error(err);
      isotope_section = true;
      continue;
    }

    if ((c>='A'&&c<='Z')||i==f.size()) {
      //Start of new symbol or end of input, record previous element if any:
      if (isotope_section)
        throw std::runtime_error(err);//should have been finished
      if (!symbol.empty()) {
        double fcount(1.0);
        if (!count.empty()) {
          char * sEnd;
          fcount = strtod(count.c_str(),&sEnd);
          if (sEnd!=(&*count.end())||fcount<=0.0)
            throw std::runtime_error(err);
        }
        if (!isotope_info.empty())
          symbol+="{"+isotope_info+"}";
        addElement(symbol.c_str(),fcount);
      } else if (!count.empty()) {
        throw std::runtime_error(err);
      }
      symbol = c;
      count.clear();
      isotope_info.clear();
    } else if (c>='a'&&c<='z') {
      if (symbol.empty())
        throw std::runtime_error(err);
      symbol += c;
    } else if ((c>='0'&&c<='9')||c=='.') {
      count += c;
    } else {
      throw std::runtime_error(err);
    }
  }
}

double IdealGas::Component::molarMass() const
{
  lock();
  if (m_mass<0) {
    if (m_elements.empty())
      throw std::runtime_error("IdealGas::Component::molarMass called on empty component");
    m_mass = 0.0;
    const unsigned n(nElements());
    for (unsigned i=0;i<n;++i)
      m_mass += molarMassByElement(i);
  }
  return m_mass;
}

unsigned IdealGas::Component::nElements() const
{
  lock();
  return m_elements.size();
}

G4Element* IdealGas::Component::element(unsigned ielement) const
{
  lock();
  if (ielement>=m_elements.size())
    throw std::runtime_error("IdealGas::Component::element index out of range");
  return m_elements[ielement].first;
}

double IdealGas::Component::molarMassByElement(unsigned ielement) const
{
  lock();
  if (ielement>=m_elements.size())
    throw std::runtime_error("IdealGas::Component::molarMassByElement index out of range");
  double mass = m_elements[ielement].first->GetA() * m_elements[ielement].second;
  mass *= CLHEP::gram / (CLHEP::Avogadro * CLHEP::amu);
  return mass;
}

const char * IdealGas::Component::name() const
{
  lock();
  return m_name.c_str();
}

G4Material * IdealGas::Component::createMaterialCalcD(double temp, double pressure, const char * the_name)
{
  lock();
  Mixture mix;
  mix.addComponent(this,1.0);
  return mix.createMaterialCalcD(temp,pressure,the_name);
}

G4Material * IdealGas::Component::createMaterialCalcP(double density, double temp, const char * the_name)
{
  lock();
  Mixture mix;
  mix.addComponent(this,1.0);
  return mix.createMaterialCalcP(density,temp, the_name);
}

G4Material * IdealGas::Component::createMaterialCalcT(double density, double pressure, const char * the_name)
{
  lock();
  Mixture mix;
  mix.addComponent(this,1.0);
  return mix.createMaterial(density,pressure, the_name);
}

G4Material * IdealGas::Component::createMaterial(double temp, double pressure, const char * the_name)
{
  lock();
  Mixture mix;
  mix.addComponent(this,1.0);
  return mix.createMaterial(temp,pressure,the_name);
}
