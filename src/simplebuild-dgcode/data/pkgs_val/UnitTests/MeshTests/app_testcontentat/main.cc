#include "Mesh/MeshFiller.hh"
#include "MeshTests/_tempworkarounds2.hh"

void testcmp(double val,double expect)
{
  if (std::fabs(val-expect)>1.0e-12)
    printf("%g (expected %g!!!)\n",val,expect);
  else
    printf("%g\n",val);
}

int main(int,char**) {

  Mesh::MeshFiller<1> m1( {3}, {0.0}, {3.0} );
  doFill(m1, 0.2, {0.5});
  doFill(m1, 1.6, {1.5});
  doFill(m1, 1.0, {2.5});
  printf("1d\n");

  testcmp( contentAt(m1,{0.6}), 0.2 );
  testcmp( contentAt(m1,{1.6}), 1.6 );
  testcmp( contentAt(m1,{2.6}), 1.0 );
  testcmp( contentAt(m1,{0.5},{1.5}), 0.9 );
  testcmp( contentAt(m1,{0.9},{1.2}), 0.2/3.+1.6*2./3. );
  testcmp( contentAt(m1,{0.9},{2.1}), 0.2/12.+1.6*10./12.+1.0/12. );
  testcmp( contentAt(m1,{-0.4},{0.6}), 0.2*0.6 );
  testcmp( contentAt(m1,{-1.0},{0.5}), 0.2/3 );
  printf("2d\n");

  Mesh::MeshFiller<2> m2( {2,2}, {0.0, -4.0}, {2.0, 0.0} );
  doFill(m2, 0.2, {0.5, -1.0});
  doFill(m2, 1.6, {1.5, -1.0});
  doFill(m2, 0.7, {0.5, -3.0});
  doFill(m2, 1.0, {1.5, -3.0});

  testcmp( contentAt(m2,{0.6,-0.9}), 0.2 );
  testcmp( contentAt(m2,{1.6,-0.9}), 1.6 );
  testcmp( contentAt(m2,{0.6,-2.9}), 0.7 );
  testcmp( contentAt(m2,{1.6,-2.9}), 1.0 );
  testcmp( contentAt(m2,{0.5,-1.0},{1.5,-3.0}), 0.6 );
  testcmp( contentAt(m2,{1.5,-1.0},{0.5,-3.0}), 1.15 );
  testcmp( contentAt(m2,{-0.5,1.0},{1.5,-3.0}), 0.2*0.5+1.0*0.25 );
  testcmp( contentAt(m2,{0.17,-5.0},{0.17,-1.2}), 0.7*2.0/3.8+0.2*0.8/3.8);

  return 0;
}
