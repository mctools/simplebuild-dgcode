#ifndef G4Interfaces_StepFilterBase_hh
#define G4Interfaces_StepFilterBase_hh

#include "Utils/ParametersBase.hh"
#include <string>

class G4Step;

namespace G4Interfaces {

  //Note, all step filters has two special boolean parameters:
  //
  // filter_negated (default false): if true, filter logic is inversed.

  class StepFilterBase : public Utils::ParametersBase {
  public:
    StepFilterBase(const char*name);
    virtual ~StepFilterBase();
    const char* getName() const;
    void dump(const char * prefix="") const;

    //This is the main method for derived classes to implement. You likely want
    //to keep implementations very efficient as it will be called for every step
    //during the G4 job:
    virtual bool filterStep(const G4Step*) const = 0;

    //reimplement if you need initialisation:
    virtual void initFilter() {};

    bool negated() const {
      if (m_negated==-1)
        m_negated = getParameterBoolean("filter_negated") ? 1 : 0;
      return m_negated;
    }
  private:
    std::string m_name;
    mutable int m_negated;
  };

}

#endif
