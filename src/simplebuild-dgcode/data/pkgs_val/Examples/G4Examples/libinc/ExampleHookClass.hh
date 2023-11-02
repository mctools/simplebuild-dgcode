#ifndef G4Examples_ExampleHookClass_hh
#define G4Examples_ExampleHookClass_hh

#include "Utils/ParametersBase.hh"

class ExampleHookClass : public Utils::ParametersBase {
public:
  ExampleHookClass();
  virtual ~ExampleHookClass();
  void prepreInitHook();
  void preInitHook();
  void postInitHook();
};

#endif
