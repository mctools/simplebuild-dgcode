#include "G4CollectFilters/StepFilterTime.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

StepFilterTime::StepFilterTime()
  : G4Interfaces::StepFilterBase("G4CollectFilters/StepFilterTime"),
    m_minPreTime(-1.0e99),
    m_maxPreTime(1.0e99),
    m_hasMinPreTime(false),
    m_hasMaxPreTime(false),
    m_pre(false),
    m_minPostTime(-1.0e99),
    m_maxPostTime(1.0e99),
    m_hasMinPostTime(false),
    m_hasMaxPostTime(false),
    m_post(false)
{
  addParameterDouble("prestep_mintime_seconds",m_minPreTime);
  addParameterDouble("prestep_maxtime_seconds",m_maxPreTime);
  addParameterDouble("poststep_mintime_seconds",m_minPostTime);
  addParameterDouble("poststep_maxtime_seconds",m_maxPostTime);
}

bool StepFilterTime::validateParameters()
{
  if (getParameterDouble("prestep_mintime_seconds")>=getParameterDouble("prestep_maxtime_seconds")) {
    printf("StepFilterTime ERROR: prestep_mintime_seconds must be less than prestep_maxtime_seconds!");
    return false;
  }
  if (getParameterDouble("poststep_mintime_seconds")>=getParameterDouble("poststep_maxtime_seconds")) {
    printf("StepFilterTime ERROR: poststep_mintime_seconds must be less than poststep_maxtime_seconds!");
    return false;
  }
  return true;
}

void StepFilterTime::initFilter()
{
  m_minPreTime = getParameterDouble("prestep_mintime_seconds");
  m_maxPreTime = getParameterDouble("prestep_maxtime_seconds");
  m_hasMinPreTime = (m_minPreTime>-1.0e99);
  m_hasMaxPreTime = (m_maxPreTime<1.0e99);
  m_minPreTime *= CLHEP::second;
  m_maxPreTime *= CLHEP::second;
  m_pre = m_hasMinPreTime || m_hasMaxPreTime;
  m_minPostTime = getParameterDouble("poststep_mintime_seconds");
  m_maxPostTime = getParameterDouble("poststep_maxtime_seconds");
  m_hasMinPostTime = (m_minPostTime>-1.0e99);
  m_hasMaxPostTime = (m_maxPostTime<1.0e99);
  m_minPostTime *= CLHEP::second;
  m_maxPostTime *= CLHEP::second;
  m_post = m_hasMinPostTime || m_hasMaxPostTime;
}

bool StepFilterTime::filterStep(const G4Step*step) const
{
  if (m_pre) {
    double t = step->GetPreStepPoint()->GetGlobalTime();
    if (m_hasMinPreTime && t < m_minPreTime)
      return false;
    if (m_hasMaxPreTime && t > m_maxPreTime)
      return false;
  }
  if (m_post) {
    printf( "filtertime: check post\n");
    double t = step->GetPostStepPoint()->GetGlobalTime();
    if (m_hasMinPostTime&& t < m_minPostTime)
      return false;
    if (m_hasMaxPostTime&& t > m_maxPostTime)
      return false;
  }
  return true;
}
