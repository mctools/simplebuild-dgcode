#include "sbldpy/sbldpy.hh"
#include "G4Examples/ExampleHooks.hh"

SBLDPY_MODULE( ExampleHooks )
{
  m.def( "prepreInitHook", &ExampleHooks::prepreInitHook );
  m.def( "preInitHook", &ExampleHooks::preInitHook );
  m.def( "postInitHook", &ExampleHooks::postInitHook );
}
