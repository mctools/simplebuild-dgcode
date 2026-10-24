#include "sbldpy/sbldpy.hh"
#include <iostream>
#include <cstdint>

class SomeClass {
public:
  SomeClass(std::int64_t nn) : m_n(nn) {}
  ~SomeClass(){}
  std::int64_t n() const { return m_n; }
private:
  std::int64_t m_n;
};

void somecppfunc()
{
  std::cout<<"in somecppfunc in BasicExamples.somemodule"<<std::endl;
}

SBLDPY_MODULE( somemodule )
{
  m.def( "somecppfunc", &somecppfunc, "This is some C++ function" );
  m.add_class<SomeClass>( "SomeClass" )
    .def_init<std::int64_t>( { "nn" } )
    .def( "n", &SomeClass::n );
}
