#include "G4Materials/G4NCUtils.hh"
#include "G4NCrystalRel/G4NCMatHelper.hh"
#include "G4NCrystalRel/G4NCManager.hh"
#include "NCrystal/NCrystal.hh"
namespace G4NC = G4NCrystalRel;
namespace NC = NCrystal;


G4Material * NCG4Utils::createPCAlloy( const std::string& name, const AlloyList& nccfgs_and_weights, double density )
{
  std::vector<std::pair<NC::MatCfg,double> > v;
  for(auto& x: nccfgs_and_weights)
    v.push_back(std::make_pair(NC::MatCfg(x.first),x.second));
  return createPCAlloy(name,v,density);
}

G4Material * NCG4Utils::createPCAlloy( const std::string& name, const AlloyListMatCfg& nccfgs_and_weights, double density )
{
  static G4NC::Manager * mgr =  G4NC::Manager::getInstance();

  double totweight=0.0;
  NC::ProcImpl::ProcComposition::ComponentList components;

  double weighted_density = 0;
  double temperature = 0;

  std::vector<std::pair<G4Material*,double> > g4mats_and_weights;
  for(auto& x: nccfgs_and_weights) {
    x.first.checkConsistency();
    if (x.second<0.0||x.second>1.0)
      throw std::runtime_error("NCG4Utils::createPCAlloy: Provided weight is outside interval (0,1]");
    totweight += x.second;
    if (temperature==0) {
      temperature = x.first.get_temp().dbl();
    } else if (temperature != x.first.get_temp().dbl()) {
      throw std::runtime_error("NCG4Utils::createPCAlloy: Temperatures not identical for different components");
    }
    G4Material * g4mat = G4NC::createMaterial(x.first);
    if (!g4mat)
      throw std::runtime_error("NCG4Utils::createPCAlloy: Problems creating G4 material from provided cfg");
    auto scmat = mgr->getScatterPropertyPtr(g4mat);
    if (!scmat)
      throw std::runtime_error("NCG4Utils::createPCAlloy: Mysteriously G4Material from G4NCrystal is without NCrystal scatter property");

    components.push_back({x.second,scmat});

    weighted_density += x.second * g4mat->GetDensity();

    g4mats_and_weights.push_back(std::make_pair(g4mat,x.second));
  }
  if (!density)
    density = weighted_density;

  if (totweight<0.99999||totweight>1.00001)
    throw std::runtime_error("NCG4Utils::createPCAlloy: Provided weights do not add to 1.0");

  G4Material * mat = new G4Material(name.c_str(),
                                    density,
                                    nccfgs_and_weights.size(),
                                    kStateSolid,
                                    temperature);

  for (auto& x : g4mats_and_weights)
    mat->AddMaterial(x.first,x.second);

  mgr->addScatterProperty(mat,NC::makeSO<NC::ProcImpl::ProcComposition>(std::move(components)));

  return mat;
}




