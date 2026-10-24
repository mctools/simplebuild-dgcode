#include "sbldpy/sbldpy.hh"
#include <iostream>

namespace {
  void somecppfunc()
  {
    std::cout<<"in somecppfunc in BasicExamples.anothermodule"<<std::endl;
  }
}

SBLDPY_MODULE( anothermodule )
{
  m.def( "somecppfunc", &somecppfunc, "This is some C++ function" );
}
