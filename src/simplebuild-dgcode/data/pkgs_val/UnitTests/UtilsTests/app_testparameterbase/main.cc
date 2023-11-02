#include "Utils/ParametersBase.hh"
#include "Utils/DummyParamHolder.hh"

class MyClassWithParameters : public Utils::ParametersBase
{
public:
  MyClassWithParameters()
  {
    addParameterDouble("some_dbl_par",17.0);
    addParameterDouble("some_dbl_par_0to1",0.5e-10,0.0,1.0);
    addParameterInt("some_int_par",-5);
    addParameterInt("some_int_par_-100to100",10,-100,100);
    addParameterBoolean("some_bool_par", true);
    addParameterString("some_string_par","some string");
    addParameterString("some_string_with_allowed_values","YES");
  }
protected:
  bool validateParameters()
  {
    const std::string& s = getParameterString("some_string_with_allowed_values");
    if (s!="YES"&&s!="NO"&&s!="MAYBE") {
      printf("ERROR: some_string_with_allowed_values must be set to one of YES, NO, MAYBE");
      return false;
    }
    if (getParameterInt("some_int_par")>getParameterInt("some_int_par_-100to100")) {
      printf("ERROR: some_int_par can not be larger than some_int_par_-100to100");
      return false;
    }
    if (getParameterInt("some_int_par_-100to100")%2!=0) {
      printf("ERROR: some_int_par_-100to100 must be even");
      return false;
    }
    return true;
  }
};

int main(int,char**)
{
  printf("---- test default values ----\n");
  MyClassWithParameters p1;
  p1.dump("  ");

  printf("---- test changed values ----\n");
  MyClassWithParameters p2;
  p2.setParameterDouble("some_dbl_par",1.0);
  p2.setParameterDouble("some_dbl_par_0to1",0.1);
  p2.setParameterInt("some_int_par",-500);
  p2.setParameterInt("some_int_par_-100to100",-100);
  p2.setParameterBoolean("some_bool_par", false);
  p2.setParameterString("some_string_par","yihaa");
  p2.setParameterString("some_string_with_allowed_values","MAYBE");
  p2.dump("  ");

  //
  char * serialised;
  unsigned serialisedLength;
  p2.serialiseParameters(serialised,serialisedLength);
  printf("---- test serialise/deserialise [%i bytes] ----\n",serialisedLength);
  Utils::DummyParamHolder dummy(serialised);
  delete[] serialised;
  dummy.dump("  ");

  return 0;
}
