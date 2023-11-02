#include "Core/Python.hh"
#include <iostream>

namespace {
  void somecppfunc()
  {
    std::cout<<"in somecppfunc in BasicExamples.anothermodule"<<std::endl;
  }
}

PYTHON_MODULE( mod )
{
  mod.def("somecppfunc", &somecppfunc, "This is some C++ function");
}
