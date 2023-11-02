#include "Core/Types.hh"
#include "Mesh/MeshFiller.hh"

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

int main(int,char**) {

  //Random tests:
  SimpleRandGen rg;
  const unsigned NDIM = 3;
  long nbins[NDIM] = {10,10,10};
  double low[NDIM] = {0.0, -10.0, 10.0 };
  double high[NDIM] = {3.0, 500.0, 2000000.0};
  Mesh::MeshFiller<NDIM,std::vector<double> > m(nbins,low,high);
  double pos0[NDIM];
  double pos1[NDIM];
  for (unsigned j = 0; j < NDIM; ++j)
    pos0[j] = pos1[j] = 0.0;

  for (unsigned i = 0; i < 100000000; ++i) {
    for (unsigned j = 0; j < NDIM; ++j) {
      double w = high[j]-low[j];
      pos0[j] += low[j] + w*(1.4*rg.shoot()-0.2);
      pos1[j] += low[j] + w*(1.4*rg.shoot()-0.2);
    }
    m.fill(1.0,pos0,pos1);
  }
  //prevent compiler optimising loop entirely away if m.fill(..) statement above
  //is commented:
  for (unsigned j = 0; j < NDIM; ++j)
    printf("%g %g\n",pos0[j],pos1[j]);
  return 0;
}
