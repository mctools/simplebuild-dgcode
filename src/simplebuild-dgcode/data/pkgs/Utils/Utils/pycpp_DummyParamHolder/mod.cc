#include "Core/Python.hh"
#include "Utils/DummyParamHolder.hh"

PYTHON_MODULE( mod )
{
  pyextra::pyimport("Utils.ParametersBase");
  py::class_<Utils::DummyParamHolder, Utils::ParametersBase >(mod, "DummyParamHolder")
    .def(py::init<const char*>())
    .def(py::init<>())
    ;
}
