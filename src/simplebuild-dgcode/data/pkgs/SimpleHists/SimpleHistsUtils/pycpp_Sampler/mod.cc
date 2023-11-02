#include "Core/Python.hh"
#include "SimpleHistsUtils/Sampler.hh"
#include "SimpleHists/Hist1D.hh"
#include <pybind11/numpy.h>

namespace SimpleHists_Sampler_py {
  void reinit_1arg(SimpleHists::Sampler* s, SimpleHists::Hist1D*h) { s->reinit(h); }

  py::object sampleMany(SimpleHists::Sampler* s, py::array_t<double> rand_vals) {
    //Replace rand_vals with sample values and return. This is because I can't
    //figure out how to create a new object without compile-time dependencies on
    //numpy and without memory leaks.
    //TODO: Use new function in NumpyUtils to create the array.
    auto n = rand_vals.size();
    double * vals = rand_vals.mutable_data();
    double * valsE = vals + n;
    for (;vals!=valsE;++vals)
      *vals = s->sample(*vals);
    return rand_vals;
  }

}

PYTHON_MODULE( mod )
{
  pyextra::pyimport("SimpleHists");
  py::class_<SimpleHists::Sampler >(mod,"Sampler")
    .def(py::init<SimpleHists::Hist1D*>())
    .def(py::init<SimpleHists::Hist1D*,double>())
    .def("sample",&SimpleHists::Sampler::sample)
    .def("sampleMany",&SimpleHists_Sampler_py::sampleMany)
    .def("__call__",&SimpleHists::Sampler::sample)
    .def("reinit",&SimpleHists::Sampler::reinit)
    .def("reinit",&SimpleHists_Sampler_py::reinit_1arg)
    ;
}
