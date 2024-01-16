#include "G4Examples/ExampleHooks.hh"
#include <iostream>

void ExampleHooks::prepreInitHook()
{
  std::cout<<std::flush<<"ExampleHooks::prepreInitHook called!"<<std::endl;
}

void ExampleHooks::preInitHook()
{
  std::cout<<std::flush<<"ExampleHooks::preInitHook called!"<<std::endl;
}

void ExampleHooks::postInitHook()
{
  std::cout<<std::flush<<"ExampleHooks::postInitHook called!"<<std::endl;
}

