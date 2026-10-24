#include "sbldpy/sbldpy.hh"
#include "G4Utils/GeoUtils.hh"
#include "Mesh/Mesh.hh"
#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4Material.hh"
#include "CLHEP/Units/SystemOfUnits.h"

#include "RandUtils/Rand.hh"//Use dedicated random stream, don't disturb G4 random stream
#include <cassert>
#include <cstdint>

void py_createDensityMap(const std::string& outfile,
                         const std::string& comments,
                         std::int64_t nsample_per_cell,
                         std::int64_t nx, std::int64_t ny, std::int64_t nz)
{
  auto tm = G4TransportationManager::GetTransportationManager();
  assert(tm);
  G4Navigator* nav = tm->GetNavigatorForTracking();
  assert(nav);
  long n[3] = {static_cast<long>(nx), static_cast<long>(ny), static_cast<long>(nz)};
  double lw[3];
  double up[3];
  G4Utils::autodetect_world_extent(lw,up);
  double dx((up[0]-lw[0])/nx);
  double dy((up[1]-lw[1])/ny);
  double dz((up[2]-lw[2])/nz);
  Mesh::Mesh<3> mesh(n,lw,up,"Density map [g/cm3]",comments.c_str());
  mesh.setCellUnits("mm");

  RandUtils::Rand rand;
  G4ThreeVector p;
  long idx[3];
  G4VPhysicalVolume* last_pv(0);
  double last_density(0.0);
  for (idx[0] = 0; idx[0] < nx; ++idx[0])
    for (idx[1] = 0; idx[1] < ny; ++idx[1])
      for (idx[2] = 0; idx[2] < nz; ++idx[2]) {
        long cellid = mesh.filler().cellIdCollapse(idx);
        double density = 0.0;
        for(long i=0; i<nsample_per_cell;++i) {
          p.set(lw[0]+(idx[0]+rand.shoot())*dx,
                lw[1]+(idx[1]+rand.shoot())*dy,
                lw[2]+(idx[2]+rand.shoot())*dz);
          G4VPhysicalVolume* pv = nav->LocateGlobalPointAndSetup(p);
          assert(pv);
          if (pv!=last_pv) {
            last_density = pv->GetLogicalVolume()->GetMaterial()->GetDensity();
            last_pv = pv;
          }
          density += last_density;
        }
        if (density)
          mesh.filler().data()[cellid] = density/(nsample_per_cell*CLHEP::gram/CLHEP::cm3);
      }
  mesh.enableStat("samples_per_cell") = nsample_per_cell;
  mesh.saveToFile(outfile);
}

SBLDPY_MODULE( DensityMap )
{
  m.def( "create_density_map", &py_createDensityMap,
         { "outfile", "comments", "nsample_per_cell", "nx", "ny", "nz" } );
}
