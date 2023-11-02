#include "Mesh/Mesh.hh"
#include "MeshTests/_tempworkarounds2.hh"

int main(int,char**) {

  long ncells[3]      = {3, 5, 8};
  double cell_low[3]  = {10.0, 10.0, 10.0 };
  double cell_high[3] = {13.0, 15.0, 18.0 };
  Mesh::Mesh<3> m(ncells,cell_low,cell_high,"fakepatterns","just a file for debugging");

  m.enableStat( "somefakestat" ) = 0.3;
  m.enableStat( "someotherfakestat" ) = -17.3;

  auto& f = m.filler();

  const double epsilon=1.0e-5;
  //0.5 in (ix,iy,iz)=(0,1,2)
  doFill(f,0.5,{10.5,11.5,12.5});//,{10.5,11.5,12.5+epsilon});
  //3.0 in (ix,iy,iz)=(1,3,5):
  doFill(f,3.0,{11.5,13.5,15.5},{11.5,13.5,15.5+epsilon});

  m.saveToFile("mesh_debug_1.mesh3d");
  return 0;
}
