#include "G4Utils/FindMaterials.hh"
#include "G4RegionStore.hh"
#include "G4Region.hh"
#include "G4Material.hh"
#include <cassert>

void G4Utils::getAllMaterialsUsedByGeometry(std::set<G4Material*>& ss)
{
  //NB: This includes base materials, i.e. might turn up materials only
  //indirectly in the geometry.
  auto region_store = G4RegionStore::GetInstance();
  for (auto itRegion = region_store->begin();itRegion!=region_store->end();++itRegion) {
    (*itRegion)->UpdateMaterialList();
    auto itMat = (*itRegion)->GetMaterialIterator();
    const unsigned nmat = (*itRegion)->GetNumberOfMaterials();
    for (unsigned imat=0;imat<nmat;++itMat,++imat) {
      assert(*itMat);
      ss.insert(*itMat);
    }
  }
}

