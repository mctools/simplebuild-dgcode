#include "G4Interfaces/ComponentToPy.hh"
#include "G4Examples/ExampleHookClass.hh"

//The ExampleHookClass object is registered in the component table (see
//G4Interfaces/ComponentHandles.hh), so it gets the usual Python interface for
//parameters (see G4Examples/ExampleHookClass.py):
SBLDPY_MODULE( _ExampleHookClass )
{
  m.def( "create_handle", []() { return G4Interfaces::registerComponent( new ExampleHookClass ); } );
  G4Interfaces::ComponentToPy<ExampleHookClass>( m, "ExampleHookClass" )
    .def( "prepreInitHook", &ExampleHookClass::prepreInitHook )
    .def( "preInitHook", &ExampleHookClass::preInitHook )
    .def( "postInitHook", &ExampleHookClass::postInitHook );
}
