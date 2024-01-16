#include "G4Utils/GeoUtils.hh"

#include "G4RegionStore.hh"
#include "G4Region.hh"
#include "G4VoxelLimits.hh"
#include "G4AffineTransform.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include <stdexcept>

namespace G4Utils {

    void autodetect_world_extent(double(&low)[3],double(&high)[3]) {
      low[0] = low[1] = low[2] = 1.0e199;
      high[0] = high[1] = high[2] = -1.0e199;
      const G4RegionStore* region_store = G4RegionStore::GetInstance();
      for (std::vector<G4Region*>::const_iterator itRegion = region_store->begin();itRegion!=region_store->end();++itRegion) {
        auto pv = (*itRegion)->GetWorldPhysical();
        if (pv) {
          //Assuming that the world volume is un-transformed (this is a requirement anyway I believe).
          auto solid = pv->GetLogicalVolume()->GetSolid();
          G4VoxelLimits vl;
          G4AffineTransform at;
          double a,b;
          if (solid->CalculateExtent(kXAxis,vl,at,a,b)) { low[0] = fmin(low[0],a); high[0] = fmax(high[0],b); }
          if (solid->CalculateExtent(kYAxis,vl,at,a,b)) { low[1] = fmin(low[1],a); high[1] = fmax(high[1],b); }
          if (solid->CalculateExtent(kZAxis,vl,at,a,b)) { low[2] = fmin(low[2],a); high[2] = fmax(high[2],b); }
        }
      }
      if (!(low[0]<high[0]&&low[0]<high[0]&&low[0]<high[0]))
        throw std::runtime_error("Could not autodetect extent of world volume");
    }

}
