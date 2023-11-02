#ifndef G4DataCollect_StepFilterTime_hh
#define G4DataCollect_StepFilterTime_hh

#include "G4Interfaces/StepFilterBase.hh"
#include "Utils/FastLookupSet.hh"
class G4LogicalTime;

class StepFilterTime : public G4Interfaces::StepFilterBase {
public:
  StepFilterTime();
  virtual ~StepFilterTime(){}
  virtual void initFilter();
  virtual bool filterStep(const G4Step*) const;
private:
  virtual bool validateParameters();
  double m_minPreTime;
  double m_maxPreTime;
  bool m_hasMinPreTime;
  bool m_hasMaxPreTime;
  bool m_pre;
  double m_minPostTime;
  double m_maxPostTime;
  bool m_hasMinPostTime;
  bool m_hasMaxPostTime;
  bool m_post;
};

#endif
