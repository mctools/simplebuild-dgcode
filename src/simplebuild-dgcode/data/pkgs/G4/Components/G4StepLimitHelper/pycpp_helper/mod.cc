#include "Core/Python.hh"
#include "G4StepLimitHelper/G4StepLimitHelper.hh"

PYTHON_MODULE( mod )
{
  py::class_<G4StepLimitHelper>(mod, "G4StepLimitHelper")
              .def("setLimit", &G4StepLimitHelper::setLimit)
              .def("addLimit", &G4StepLimitHelper::addLimit)
              .def("setWorldLimit", &G4StepLimitHelper::setWorldLimit)
              .def("addWorldLimit", &G4StepLimitHelper::addWorldLimit)
              ;

}
