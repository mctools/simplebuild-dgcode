#include "Core/Python.hh"
#include "G4Examples/ExampleHookClass.hh"

PYTHON_MODULE( mod )
{
  py::class_<ExampleHookClass,Utils::ParametersBase>(mod,"ExampleHookClass")
    .def( py::init<>() )
    .def("prepreInitHook",&ExampleHookClass::prepreInitHook)
    .def("preInitHook",&ExampleHookClass::preInitHook)
    .def("postInitHook",&ExampleHookClass::postInitHook)
    ;
}
