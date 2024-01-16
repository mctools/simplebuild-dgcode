#include "Core/Python.hh"
#include <pybind11/numpy.h>

namespace UtilsTests {
  py::object gensquares() {
    std::size_t n = 100000;
    auto o = py::array_t<double>(n);
    double * buf = o.mutable_data();
    for (std::size_t i = 0;i<n;++i)
      buf[i] = i*i;
    return o;
  }
}


PYTHON_MODULE( mod )
{
  mod.def("gensquares",UtilsTests::gensquares);
}
