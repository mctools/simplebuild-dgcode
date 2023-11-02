#include "Core/Python.hh"
#include <iostream>

class SomeClass {
public:
  SomeClass(int nn) : m_n(nn) {}
  ~SomeClass(){}
  int n() const { return m_n; }
private:
  int m_n;
};

void somecppfunc()
{
  std::cout<<"in somecppfunc in BasicExamples.somemodule"<<std::endl;
}

PYTHON_MODULE( mod )
{
  mod.def("somecppfunc", somecppfunc, "This is some C++ function");
  py::class_<SomeClass>(mod,"SomeClass")
    .def(py::init<int>())
    .def("n",&SomeClass::n)
    ;
}
