#include "sbldpy/sbldpy.hh"
#include <vector>

namespace UtilsTests {
  sbldpy::NumpyCopy<double> gensquares() {
    std::size_t n = 100000;
    std::vector<double> v(n);
    for (std::size_t i = 0;i<n;++i)
      v[i] = i*i;
    return v;
  }
}

SBLDPY_MODULE( usenp )
{
  m.def( "gensquares", &UtilsTests::gensquares );
}
