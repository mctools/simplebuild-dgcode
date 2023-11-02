//See IdealGasBuilder.hh for general information.
//Author: Thomas Kittelmann, January 2015.

#include "IdealGasBuilder/IdealGasMixture.hh"
#include "IdealGasBuilder/IdealGasComponent.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <map>

IdealGas::Mixture::Mixture(const char * formula)
  : m_by_mass_fractions(false)
{
  addComponents(formula);
}

IdealGas::Mixture::Mixture()
  : m_by_mass_fractions(false)
{
}

IdealGas::Mixture::~Mixture()
{
  for (auto it = m_owned_components.begin(); it!=m_owned_components.end(); ++it)
    delete *it;
}

void IdealGas::Mixture::addComponents(const char* formula)
{
  const char * err = "IdealGas::Mixture::addComponents error in formula";
  std::string f(formula);
  std::string::size_type n = f.size();
  bool fractions_by_mass(false);
  if (f.size()>8&&strncmp(&f[n-8],"{bymass}",8)==0) {
    n -= 8;
    fractions_by_mass = true;
  }
  if (n==0)
    throw std::runtime_error(err);

  if (f[0]>='A'&&f[0]<='Z') {
    //Either a bad formula, or a single-component formula which needs "1.0*'
    //prepended to work with the generic code below:
    f = "1.0*" + f;
    n += 4;
  }
  std::string comp_formula;
  std::string fraction;
  bool fracmode(true);
  for (unsigned i=0;i<=n;++i) {
    char c = i<n ? f[i] : '#';
    if (c=='{') {
      if (fracmode || comp_formula.empty() || fraction.empty() || i+1==n)
        throw std::runtime_error(err);
      comp_formula+='{';
      while (++i,i<n&&f[i]!='}') {
        comp_formula += f[i];
      }
      if (i==n||f[i]!='}')
        throw std::runtime_error(err);
      comp_formula+='}';
    } else if ((c=='+'&&!fracmode)||i==n) {
      if (fracmode||comp_formula.empty()||fraction.empty())
        throw std::runtime_error(err);
      char * sEnd;
      double ffraction = strtod(fraction.c_str(),&sEnd);
      if (sEnd!=(&*fraction.end())||ffraction<0.0||ffraction>1.0)
        throw std::runtime_error(err);
      if (fractions_by_mass)
        addComponentFractionByMass(comp_formula.c_str(),ffraction);
      else
        addComponent(comp_formula.c_str(),ffraction);
      comp_formula.clear();
      fraction.clear();
      fracmode=true;
    } else if (c=='*') {
      if (!fracmode)
        throw std::runtime_error(err);
      fracmode=false;
    } else {
      bool isdigit( (c>='0'&&c<='9') || c=='.' || ( fracmode && ( c=='e'||c=='E'||c=='+' || c=='-') ) );
      if (fracmode) {
        if (!isdigit)
          throw std::runtime_error(err);
        fraction += c;
      } else {
        bool isalpha((c>='A'&&c<='Z')||(c>='a'&&c<='z'));
        if (!isdigit && !isalpha)
          throw std::runtime_error(err);
        comp_formula += c;
      }
    }
  }
}

void IdealGas::Mixture::addComponentFractionByMass( Component* c, double fraction, bool manage )
{
  if (!m_by_mass_fractions&&!m_components.empty())
    throw std::runtime_error("IdealGas::Mixture::addComponentFractionByMass can't mix fractions-by-volume and fractions-by-mass");
  m_by_mass_fractions=true;
  if (fraction<=0||fraction>1)
    throw std::runtime_error("IdealGas::Mixture::addComponentFractionByMass requires 0.0<fraction<=1.0");
  assert(c);
  m_components.push_back(std::make_pair(c,fraction));
  if (manage)
    m_owned_components.push_back(c);
}

void IdealGas::Mixture::addComponentFractionByMass( Component& c, double fraction, bool manage )
{
  addComponent(&c,fraction,manage);
}

void IdealGas::Mixture::addComponentFractionByMass(const char* formula,double fraction)
{
  Component * c = new Component(formula);
  m_owned_components.push_back(c);
  addComponentFractionByMass(c,fraction);
}

void IdealGas::Mixture::addComponent(const char * formula,double fraction)
{
  Component * c = new Component(formula);
  m_owned_components.push_back(c);
  addComponent(c,fraction);
}

void IdealGas::Mixture::addComponent(Component& c,double fraction, bool manage)
{
  addComponent(&c,fraction,manage);
}

void IdealGas::Mixture::addComponent(Component* c,double fraction, bool manage)
{
  if (m_by_mass_fractions)
    throw std::runtime_error("IdealGas::Mixture::addComponent can't mix fractions-by-volume and fractions-by-mass");
  if (fraction<=0||fraction>1)
    throw std::runtime_error("IdealGas::Mixture::addComponent requires 0.0<fraction<=1.0");
  assert(c);
  m_components.push_back(std::make_pair(c,fraction));
  if (manage)
    m_owned_components.push_back(c);
}

void IdealGas::Mixture::calcDensities(double& temp, double& pressure, double& density,
                                      std::map<G4Element*,double>& element2density) const
{
  if (m_components.empty())
    throw std::runtime_error("IdealGas::Mixture::createMaterialXXX called before any components were added");
  if (temp<0)
    temp = CLHEP::STP_Temperature;
  if (pressure<0)
    pressure = CLHEP::STP_Pressure;
  density = 0.0;

  const double R = CLHEP::k_Boltzmann* CLHEP::Avogadro;
  const double moles_per_vol_total = pressure / ( R * temp);
  auto itE = m_components.end();
  double mass_avr(0);
  if (m_by_mass_fractions) {
    //Need to translate by-mass fractions (fm_i) to by-vol fractions
    //(fv_i). This is done by calculating the average mass of a molecule for the
    //whole gas (mass_avr), and using fv_i = (fm_i/m_i) * mass_avr.
    double tmp(0);
    for (auto it = m_components.begin();it!=itE;++it)
      tmp += ( it->second / it->first->molarMass() );
    mass_avr = 1/tmp;
  }

  double fraction_sum(0.0);
  for (auto it = m_components.begin();it!=itE;++it) {
    fraction_sum += it->second;
    double component_moles_per_vol = it->second * moles_per_vol_total;
    if (m_by_mass_fractions)
      component_moles_per_vol *= ( mass_avr / it->first->molarMass() );
    double component_density = it->first->molarMass() * component_moles_per_vol;
    density += component_density;

    const unsigned nEle(it->first->nElements());
    for (unsigned i=0;i<nEle;++i) {
      G4Element * element = it->first->element(i);
      double density_element = it->first->molarMassByElement(i) * component_moles_per_vol;
      auto itEle = element2density.find(element);
      if (itEle == element2density.end()) element2density[element] = density_element;
      else itEle->second += density_element;
    }
  }
  if (fabs(fraction_sum-1.0)>1.0e-9)
    throw std::runtime_error("IdealGas::Mixture::createMaterialXXX component fractions do not sum to 1.0");
}

G4Material * IdealGas::Mixture::buildMat(const char * name, double temp, double pressure, double density,
                                         const std::map<G4Element*,double>& element2density, double scale_e2d) const
{
  auto mat = new G4Material(name,density,element2density.size(),
                            kStateGas,temp,pressure);
  assert(density>0.0);

  //guard against rounding errors which triggers G4 exceptions:
  double frac_tot = 0.0;
  for (auto it = element2density.begin();it!=element2density.end();++it)
    frac_tot += (it->second / density) * scale_e2d;
  assert(std::abs<double>(frac_tot-1.0)<1.0e-10);

  //Reorder in different map with element names as keys for reproducibility so
  //order of addElement calls won't depend on pointer sorting (at least not when
  //element names are different):

  std::map<std::pair<G4String,G4Element*>,std::pair<G4Element*,double> > name2ed;
  for (auto it = element2density.begin();it!=element2density.end();++it)
    name2ed[std::make_pair(it->first->GetName(),it->first)] = *it;

  for (auto it = name2ed.begin();it!=name2ed.end();++it) {
    double f = (it->second.second / density) * scale_e2d;
    f /= frac_tot;//important for numerical stability reasons to perform the division with frac_tot here
    mat->AddElement(it->second.first, f);
  }
  return mat;
}

G4Material * IdealGas::Mixture::createMaterial(double temp, double pressure,const char * name)
{
  return createMaterialCalcD(temp,pressure,name);
}

G4Material * IdealGas::Mixture::createMaterialCalcD(double temp, double pressure,const char * name)
{
  double density;
  std::map<G4Element*,double> element2density;
  calcDensities(temp,pressure,density,element2density);
  std::string thename(name?name:"");
  if (thename.empty()) {
    std::ostringstream autoname;
    autoname << "IdealGasMix[" << autoName() << "/"<<temp/CLHEP::kelvin<<"K" << "/"<<pressure/CLHEP::bar<<"bar]";
    thename = autoname.str();
  }
  return buildMat(thename.c_str(),temp,pressure,density,element2density,1.0);
}

G4Material * IdealGas::Mixture::createMaterialCalcP(double density, double temp,const char * name)
{
  if (density<=0)
    throw std::runtime_error("IdealGas::Mixture::createMaterialCalcP density must be positive");

  //First calculate density at STP pressure, then scale resulting pressure to get desired density.
  double pressure = -1;
  double densityCalc;
  std::map<G4Element*,double> element2density;
  calcDensities(temp,pressure,densityCalc,element2density);
  double scale = density / densityCalc;
  std::string thename(name?name:"");
  if (thename.empty()) {
    std::ostringstream autoname;
    autoname << "IdealGasMix[" << autoName() << "/"<<density/(CLHEP::kg/CLHEP::m3)<<"kgm-3/"<<temp/CLHEP::kelvin<<"K]";
    thename = autoname.str();
  }
  return buildMat(thename.c_str(),temp,pressure*scale,density,element2density,scale);
}

G4Material * IdealGas::Mixture::createMaterialCalcT(double density, double pressure,const char * name)
{
  if (density<=0)
    throw std::runtime_error("IdealGas::Mixture::createMaterialCalcT density must be positive");
  //First calculate density at STP temperature, then scale resulting temperature to get desired density.
  double temp = -1;
  double densityCalc;
  std::map<G4Element*,double> element2density;
  calcDensities(temp,pressure,densityCalc,element2density);
  double scale = density / densityCalc;
  std::string thename(name?name:"");
  if (thename.empty()) {
    std::ostringstream autoname;
    autoname << "IdealGasMix[" << autoName() << "/"<<density/(CLHEP::kg/CLHEP::m3)<<"kgm-3/"<<pressure/CLHEP::bar<<"bar]";
    thename = autoname.str();
  }
  return buildMat(thename.c_str(),temp/scale,pressure,density,element2density,scale);
}

std::string IdealGas::Mixture::autoName() const
{
  if (m_components.size()==1)
    return m_components.begin()->first->name();
  std::ostringstream an;
  auto itE = m_components.end();
  for (auto it = m_components.begin();it!=itE;++it) {
    if (it!=m_components.begin())
      an <<"+";
    an << it->second << "*" << it->first->name();
  }
  if (m_by_mass_fractions)
    an << "{bymass}";
  return an.str();
}
