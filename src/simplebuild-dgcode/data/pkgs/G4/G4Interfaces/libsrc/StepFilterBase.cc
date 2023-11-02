#include "G4Interfaces/StepFilterBase.hh"

G4Interfaces::StepFilterBase::StepFilterBase(const char*name)
  : Utils::ParametersBase(),
    m_name(name),
    m_negated(-1)
{
  addParameterBoolean("filter_negated",false);
}

const char* G4Interfaces::StepFilterBase::getName() const
{
  return m_name.c_str();
}

G4Interfaces::StepFilterBase::~StepFilterBase()
{
}

void G4Interfaces::StepFilterBase::dump(const char * prefix) const
{
  printf("%sStepFilter[%s]:\n",prefix,m_name.c_str());
  std::string p = prefix;
  p+="  ";
  Utils::ParametersBase::dump(p.c_str());
}
