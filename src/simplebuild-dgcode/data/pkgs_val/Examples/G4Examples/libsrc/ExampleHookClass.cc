#include "G4Examples/ExampleHookClass.hh"

ExampleHookClass::ExampleHookClass()
{
  printf("ExampleHookClass::constructor\n");
  addParameterInt("some_param_int",0);
  addParameterDouble("some_param_dbl",17.0);
}

ExampleHookClass::~ExampleHookClass()
{
  //fixme: not sure to be called!
  printf("ExampleHookClass::destructor\n");
}

void ExampleHookClass::prepreInitHook()
{
  printf("ExampleHookClass::prepreInitHook called (pars are %i and %g)\n",
         getParameterInt("some_param_int"),
         getParameterDouble("some_param_dbl"));
}

void ExampleHookClass::preInitHook()
{
  printf("ExampleHookClass::preInitHook called (pars are %i and %g)\n",
         getParameterInt("some_param_int"),
         getParameterDouble("some_param_dbl"));
}

void ExampleHookClass::postInitHook()
{
  printf("ExampleHookClass::postInitHook called (pars are %i and %g)\n",
         getParameterInt("some_param_int"),
         getParameterDouble("some_param_dbl"));
}
