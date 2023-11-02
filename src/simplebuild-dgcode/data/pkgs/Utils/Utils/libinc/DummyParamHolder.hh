#ifndef Utils_DummyParamHolder_hh
#define Utils_DummyParamHolder_hh

//A ParametersBase implementation which can be used to represent deserialised
//parameters in an object oriented manner.

#include "Utils/ParametersBase.hh"

namespace Utils {

  class DummyParamHolder : public ParametersBase {
  public:
    DummyParamHolder(const char* data);//deserialise and lock
    DummyParamHolder();//construct empty unlocked instance
    virtual ~DummyParamHolder();
  };

}

#endif
