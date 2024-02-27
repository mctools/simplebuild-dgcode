#include "Core/Types.hh"
#include "Mesh/MeshFiller.hh"
#include "Utils/DelayedAllocVector.hh"
#include <cstdlib>
#include "MeshTests/_tempworkarounds2.hh"

namespace {

  template <class T> double expected_precision() { return 2.0e-12; }
  //unused?!? template <> double expected_precision<float>() { return 1.0e-7; }

  class SimpleRandGen {
    // very simple multiply-with-carry rand gen
    // (http://en.wikipedia.org/wiki/Random_number_generation)
  public:
    SimpleRandGen()
      : m_w(117),/* must not be zero, nor 0x464fffff */
        m_z(11713)/* must not be zero, nor 0x9068ffff */
    {
    }
    ~SimpleRandGen(){}
    double shoot()
    {
      m_w = 18000 * (m_w & 65535) + (m_w >> 16);
      m_z = 36969 * (m_z & 65535) + (m_z >> 16);
      return double((m_z << 16) + m_w)/double(UINT32_MAX);  /* 32-bit result */
    }
  private:
    std::uint32_t m_w;
    std::uint32_t m_z;
  };

  template <int NDIM,class TStorage>
  void test_and_dump(Mesh::MeshFiller<NDIM,TStorage>& m)
  {
    long cell[NDIM];
    for (long i = 0; i < m.nCells(); ++i) {
      m.cellIdExpand(i,cell);
      if (m.cellIdCollapse(cell)!=i) {
        printf("Bad cell ID mappings");
        exit(1);
      }
    }
    m.dump();
  }

  template <class TStorage>
  void test_1d(int nbins, double low, double high, double ray_low, double ray_high)
  {
    printf("new test1d\n");
    Mesh::MeshFiller<1,TStorage> m1( {nbins}, {low}, {high} );//real 1D
    Mesh::MeshFiller<2,TStorage> m2( {nbins,1}, {low,0.0}, {high,1.0} );//pseudo 1D
    Mesh::MeshFiller<2,TStorage> m2b( {1,nbins}, {0.0,low}, {1.0,high} );//pseudo 1D
    Mesh::MeshFiller<3,TStorage> m3( {nbins,1,1}, {low,0.0,0.0}, {high,1.0,1.0} );//pseudo 1D
    Mesh::MeshFiller<3,TStorage> m3b( {1,nbins,1}, {0.0,low,0.0}, {1.0,high,1.0} );//pseudo 1D

    double dep = 1.0;

    doFill(m1,dep, {ray_low}, {ray_high});
    doFill(m2,dep, {ray_low,0.5}, {ray_high,0.5});
    doFill(m2b,dep, {0.5,ray_low}, {0.5,ray_high});
    doFill(m3,dep, {ray_low,0.5,0.5}, {ray_high,0.5,0.5});
    doFill(m3b,dep, {0.5,ray_low,0.5}, {0.5,ray_high,0.5});

    double raymin(ray_low),raymax(ray_high);
    if (raymin>raymax)
      std::swap(raymin,raymax);

    //test all bins:
    for (int ibin = 0; ibin < nbins; ++ibin) {
      double bin_low = low + ibin * (high-low)/nbins;
      double bin_high = low + (ibin+1) * (high-low)/nbins;
      //skip some obvious zero bins in dbg build to keep test time reasonable
      if (nbins > 1000000 && ibin > 100 && ibin < nbins - 100 && (bin_high < raymin || bin_low > raymax) )
        continue;
      double expect = dep * (std::max(raymin,std::min(bin_high,raymax))-std::min(raymax,std::max(bin_low,raymin)))/(raymax-raymin);
      expect = static_cast<typename TStorage::value_type>(expect);
      long temp[1];//workaround gcc 4.9.2 segfaulting on m1.cellContent({ibin}) !!
      temp[0] = ibin;
      double got = m1.cellContent(temp);
      double got2 = getCellContent(m2,{ibin,0});
      double got2b = getCellContent(m2b,{0,ibin});
      double got3 = getCellContent(m3,{ibin,0,0});
      double got3b = getCellContent(m3b,{0,ibin,0});
      if (got!=got2||got!=got2b||got!=got3||got!=got3b) {
        printf("Unexpected contents in ND versus 1D (%g vs %g vs %g vs %g vs %g)\n",got,got2,got2b,got3,got3b);
        exit(1);
      }
      if (nbins<1000||got||expect)
        printf("found in bin %i : %g (expected %g)\n",ibin,got,expect);
      if (std::fabs(got-expect)>expected_precision<TStorage>()) {
        printf("Unexpected contents!\n");
        exit(1);
      }
    }
  }

  template <class TStorage>
  void test_2d(int nbinsx, double lowx, double highx,
               int nbinsy, double lowy, double highy,
               double ray_lowx, double ray_lowy, double ray_highx, double ray_highy)
  {
    printf("new test2d\n");
    Mesh::MeshFiller<2,TStorage> m2( {nbinsx,nbinsy}, {lowx,lowy}, {highx,highy} );

    double dep = 1.0;

    doFill(m2,dep, {ray_lowx,ray_lowy}, {ray_highx,ray_highy});

    double ray_dx = ray_highx - ray_lowx;
    double ray_dy = ray_highy - ray_lowy;
    double ray_inv_x = ray_dx ? 1.0/ray_dx : std::numeric_limits<double>::infinity();
    double ray_inv_y = ray_dy ? 1.0/ray_dy : std::numeric_limits<double>::infinity();

    //test all bins:
    for (int ibinx = 0; ibinx < nbinsx; ++ibinx) {
      double bin_lowx = lowx + ibinx * (highx-lowx)/nbinsx;
      double bin_highx = lowx + (ibinx+1) * (highx-lowx)/nbinsx;
#ifndef NDEBUG
      //skip some obvious zero bins in dbg build to keep test time reasonable
      if (ibinx > 100 && ibinx < nbinsx - 100 && bin_highx < std::min<double>(ray_lowx,ray_highx))
        continue;
#endif
      for (int ibiny = 0; ibiny < nbinsy; ++ibiny) {
        double bin_lowy = lowy + ibiny * (highy-lowy)/nbinsy;
        double bin_highy = lowy + (ibiny+1) * (highy-lowy)/nbinsy;
#ifndef NDEBUG
        //skip some obvious zero bins in dbg build to keep test time reasonable
        if (ibiny > 100 && ibiny < nbinsy - 100 && bin_highy < std::min<double>(ray_lowy,ray_highy))
          continue;
#endif
        //figure out how much of the ray intersects this bin:
        double tmin(0.0), tmax(1.0);
        //constrain x:
        double t1 = (bin_lowx - ray_lowx) * ray_inv_x;
        double t2 = (bin_highx - ray_lowx) * ray_inv_x;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
        //constrain y:
        t1 = (bin_lowy - ray_lowy) * ray_inv_y;
        t2 = (bin_highy - ray_lowy) * ray_inv_y;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
        double expect = dep * ( tmax>tmin ? dep * (tmax-tmin) : 0.0 );

        double got = getCellContent(m2,{ibinx,ibiny});
        if (nbinsx*nbinsy<1000||got||expect)
          printf("found in bin (%i,%i) : %g (expected %g)\n",ibinx,ibiny,got,expect);
        if (std::fabs(got-expect)>expected_precision<TStorage>()) {
          printf("Unexpected contents!\n");
          exit(1);
        }
      }
    }
  }

  template <class TStorage>
  void many_1d_tests()
  {
    test_1d<TStorage>(10, 0.0, 10.0, 0.0, 10.0);
    test_1d<TStorage>(10, 0.0, 10.0, 10.0, 0.0);
    test_1d<TStorage>(10, 0.0, 10.0, -5.0, -3.0);
    test_1d<TStorage>(10, 0.0, 10.0, -3.0, -5.0);
    test_1d<TStorage>(10, 0.0, 10.0, 0.8, 1.4);
    test_1d<TStorage>(10, 0.0, 10.0, 1.4, 0.8);
    test_1d<TStorage>(10, 0.0, 10.0, -0.5, 1.5);
    test_1d<TStorage>(10, 0.0, 10.0, 1.5, -0.5);
    test_1d<TStorage>(10, 0.0, 10.0, -5,15);
    test_1d<TStorage>(10, 0.0, 10.0, 8.5, 10.5);
    test_1d<TStorage>(10, 0.0, 10.0, 2.2, 2.4);
    test_1d<TStorage>(10, 0.0, 10.0, 2.0+0.1, 3.0+0.1);
    test_1d<TStorage>(10, 0.0, 10.0, 2.0+0.0001, 3.0+0.0001);
    test_1d<TStorage>(10, 0.0, 10.0, 2.0+1e-6, 3.0+1e-6);
    const long bignum = 10000000;
    test_1d<TStorage>(bignum, 0.0, 1e7, 2.0+1e-6, 3.0+1e-6);
    test_1d<TStorage>(bignum, 0.0, 1e7, (bignum-3)+1.0e-6, (bignum-2)+1.0e-6);
    test_1d<TStorage>(bignum, 0.0, 1e7, (bignum-3)+1.0e-7, (bignum-2)+1.0e-7);
    test_1d<TStorage>(bignum, 0.0, 1e7, (bignum-3)+1.0e-8, (bignum-2)+1.0e-8);
    test_1d<TStorage>(bignum, 0.0, 1e7, (bignum-3)+1.0e-9, (bignum-2)+1.0e-9);
  }

  template <class TStorage>
  void test_3d(int nbinsx, double lowx, double highx,
               int nbinsy, double lowy, double highy,
               int nbinsz, double lowz, double highz,
               double ray_lowx, double ray_lowy, double ray_lowz,
               double ray_highx, double ray_highy, double ray_highz,
               bool verbose=true)
  {
    if (verbose)
      printf("new test3d\n");
    Mesh::MeshFiller<3,TStorage> m3( {nbinsx,nbinsy,nbinsz}, {lowx,lowy,lowz}, {highx,highy,highz} );

    double dep = 1.0;

    doFill(m3,dep, {ray_lowx,ray_lowy,ray_lowz}, {ray_highx,ray_highy,ray_highz});

    double ray_dx = ray_highx - ray_lowx;
    double ray_dy = ray_highy - ray_lowy;
    double ray_dz = ray_highz - ray_lowz;
    double ray_inv_x = ray_dx ? 1.0/ray_dx : std::numeric_limits<double>::infinity();
    double ray_inv_y = ray_dy ? 1.0/ray_dy : std::numeric_limits<double>::infinity();
    double ray_inv_z = ray_dz ? 1.0/ray_dz : std::numeric_limits<double>::infinity();

    //test all bins:
    for (int ibinx = 0; ibinx < nbinsx; ++ibinx) {
      double bin_lowx = lowx + ibinx * (highx-lowx)/nbinsx;
      double bin_highx = lowx + (ibinx+1) * (highx-lowx)/nbinsx;
      for (int ibiny = 0; ibiny < nbinsy; ++ibiny) {
        double bin_lowy = lowy + ibiny * (highy-lowy)/nbinsy;
        double bin_highy = lowy + (ibiny+1) * (highy-lowy)/nbinsy;
        for (int ibinz = 0; ibinz < nbinsz; ++ibinz) {
          double bin_lowz = lowz + ibinz * (highz-lowz)/nbinsz;
          double bin_highz = lowz + (ibinz+1) * (highz-lowz)/nbinsz;

          //figure out how much of the ray intersects this bin:
          double tmin(0.0), tmax(1.0);
          //constrain x:
          double t1 = (bin_lowx - ray_lowx) * ray_inv_x;
          double t2 = (bin_highx - ray_lowx) * ray_inv_x;
          tmin = std::max(tmin, std::min(t1, t2));
          tmax = std::min(tmax, std::max(t1, t2));
          //constrain y:
          t1 = (bin_lowy - ray_lowy) * ray_inv_y;
          t2 = (bin_highy - ray_lowy) * ray_inv_y;
          tmin = std::max(tmin, std::min(t1, t2));
          tmax = std::min(tmax, std::max(t1, t2));
          //constrain z:
          t1 = (bin_lowz - ray_lowz) * ray_inv_z;
          t2 = (bin_highz - ray_lowz) * ray_inv_z;
          tmin = std::max(tmin, std::min(t1, t2));
          tmax = std::min(tmax, std::max(t1, t2));
          double expect = dep * ( tmax>tmin ? dep * (tmax-tmin) : 0.0 );

          double got = getCellContent(m3,{ibinx,ibiny,ibinz});
          if (verbose&&(nbinsx*nbinsy*nbinsz<1000||got||expect))
            printf("found in bin (%i,%i,%i) : %g (expected %g)\n",ibinx,ibiny,ibinz,got,expect);
          if (std::fabs(got-expect)>expected_precision<TStorage>()) {
            printf("Unexpected contents!\n");
            exit(1);
          }
        }
      }
    }
  }
}

int main(int,char**) {

  //1D test:
  Mesh::MeshFiller<1> m1( {10L}, {0.0}, {10.0} );
  doFill(m1,17.0, {4.5});
  test_and_dump(m1);

  //  m1.reset();
  //reset contents by directly accessing and manipulating data storage:
  std::memset(&m1.data()[0],0,sizeof(double)*m1.data().size());

  doFill(m1,1.0,{-5.0},{-4.0});
  assert(m1.cellContent(0)==0.0);

  many_1d_tests<Utils::DelayedAllocVector<double> >();
  many_1d_tests<Utils::DelayedAllocVector<float> >();
  test_1d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,0.5,1.5);

  //2D test:
  Mesh::MeshFiller<1> m2( {10}, {0.0}, {10.0} );
  doFill(m2,17.0, {4.5});
  test_and_dump(m2);

  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    0.0, 0.0,    4.0, 4.0);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    4.0, 4.0,    0.0, 0.0);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    3.0, 3.0,    0.0, 0.0);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    0.0, 0.0,    3.0, 3.0);
  test_2d<Utils::DelayedAllocVector<float> >(3, 0.0, 3.0,   3, 0.0, 3.0,    0.0, 0.0,    4.0, 4.0);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    -1.0, -1.0,    4.0, 4.0);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    4.0, 4.0,    -1.0, -1.0);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    0.0, 1.0,    4.0, 5.0);
  test_2d<Utils::DelayedAllocVector<float> >(3, 0.0, 3.0,   3, 0.0, 3.0,    0.0, 1.0,    4.0, 5.0);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    0.0, 0.0+1e-6,1.0, 1.0+1e-6);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    0.0, 0.0+1e-9,1.0, 1.0+1e-9);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    0.0, 0.0+0.2e-9,1.0, 1.0+0.2e-9);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    0.0, 0.0+0.2e-11,1.0, 1.0+0.2e-11);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,   3, 0.0, 3.0,    2.0, 0.5,    3.0, 1.0+1.0e-13);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0, 3, 0.0, 3.0, 0.5,2.5, 1.5,1.5);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0, 3, 0.0, 3.0, 1.5,1.5, 0.5,2.5);
  test_2d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,2, 0.0, 2.0,2.5, 1.0,-0.5,1.0-1e-13);
  const long bignum = 10000000;
  test_2d<Utils::DelayedAllocVector<double> >(bignum, 0.0, 1e7,   2, 0.0, 2.0, (bignum-1)-1.0e-9, 0.9, (bignum-1)+1.0e-9, 0.1);

  //3D test:
  Mesh::MeshFiller<3> m3( {   3,   3,    2 },
                          { 0.0, 0.0, -0.5 },
                          { 3.0, 3.0,  0.5 } );
  doFill(m3,17.0, {2.5,2.5,0.1});
  test_and_dump(m3);


  Mesh::MeshFiller<3> m3_2( {     3,      4,    5 },
                            { -10.0, -100.0,  0.0 },
                            {  20.0,  -60.0,  5.0 } );

  test_3d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0, 3, 0.0, 3.0, 3, 0.0, 3.0,
                  0.0,0.0,0.0,3.0,3.0,3.0);

  test_3d<Utils::DelayedAllocVector<double> >(3, 0.0, 30.0,3, -1.0, 2.0,3, 0.0, 300.0,
                  0.0,-1.0,0.0,30.0,2.0,300.0);

  test_3d<Utils::DelayedAllocVector<double> >(3, 0.0, 30.0,3, -1.0, 2.0,3, 0.0, 300.0,
                  0.0,-1.0,0.0,30.0,2.0,300.0);

  test_3d<Utils::DelayedAllocVector<double> >(3, 0.0, 30.0,3, -1.0, 2.0,3, 0.0, 300.0,
                  15.0,-1.0,0.0,19.0,2.0,300.0);

  test_3d<Utils::DelayedAllocVector<double> >( 3, 0.0, 3.0, 5, 0.0, 5.0, 2, 0.0, 2.0,
                   1.0,2.5, 1.0, 1.0,-0.5, 0.99);

  //Random tests:
  SimpleRandGen rg;
  double lowx(0.0), highx(3.0);
  double lowy(-10.0), highy(500.0);
  double lowz(10.0), highz(2000000.0);
  test_3d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,
                  1, 0.0, 5.0,
                  1, 0.0, 2.0,
                  0.5,2.0,1.0,
                  1.5,2.0,1.0);
  test_3d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,
                  5, 0.0, 5.0,
                  1, 0.0, 1,
                  0.5,2.5,0.5,
                  1.5,1.5,0.5);
  test_3d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,
                  5, 0.0, 5.0,
                  2, 0.0, 2,
                  0.391406,2.01724,1.00035,1.03088,1.81163,0.299311);

  test_3d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,
                  5, -10.0, 500.0,
                  2, 10.0, 2000000.0,
                  1.1974298082053263,183.647333094768044,1000099.48819034838,
                  -0.25680433787796747,-51.2958857378214361,999757.917455491843);

  test_3d<Utils::DelayedAllocVector<double> >(3, 0.0, 3.0,
                  5, -10.0, 500.0,
                  2, 10.0, 2000000.0,
                  0.511938724320367466,12.1023464454576164,999947.032623409876,
                  1.23031798457501429,69.2652530607919346,1000039.60948389641);


  double dx(highx-lowx);
  double dy(highx-lowx);
  double dz(highx-lowx);
  for (unsigned i = 0; i < 1001; ++i) {
    if (i%100==0)
      printf("randomtest %i\n",i);
    double rayl_x = lowx + dx * (1.4*rg.shoot()-0.2);
    double rayl_y = lowy + dy * (1.4*rg.shoot()-0.2);
    double rayl_z = lowz + dz * (1.4*rg.shoot()-0.2);
    double rayh_x = lowx + dx * (1.4*rg.shoot()-0.2);
    double rayh_y = lowy + dy * (1.4*rg.shoot()-0.2);
    double rayh_z = lowz + dz * (1.4*rg.shoot()-0.2);
     //  printf("Ray: (%.18g,%.18g,%.18g) -> (%.18g,%.18g,%.18g)\n",
     //         rayl_x,rayl_y,rayl_z,
     //         rayh_x,rayh_y,rayh_z);

    test_3d<Utils::DelayedAllocVector<double> >(3, lowx, highx,
                                                5, lowy, highy,
                                                2, lowz, highz,
                                                rayl_x,rayl_y,rayl_z,
                                                rayh_x,rayh_y,rayh_z,
                                                false);
  }

  return 0;
}
