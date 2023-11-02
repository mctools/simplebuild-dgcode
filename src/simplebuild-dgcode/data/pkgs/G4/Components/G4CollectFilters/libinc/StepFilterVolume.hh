#ifndef G4DataCollect_StepFilterVolume_hh
#define G4DataCollect_StepFilterVolume_hh

#include "G4Interfaces/StepFilterBase.hh"
#include "Utils/FastLookupSet.hh"
class G4LogicalVolume;

class StepFilterVolume : public G4Interfaces::StepFilterBase {
public:
  StepFilterVolume();
  virtual ~StepFilterVolume(){}
  virtual void initFilter();
  virtual bool filterStep(const G4Step*) const;
private:
  virtual bool validateParameters();
  Utils::FastLookupSet<std::string> m_volnames;//todo/fixme: should use fast string sort!!!!
  unsigned m_volnames_acceptcount;
  mutable G4LogicalVolume * m_lastvol;
  mutable bool m_lastvolresult;
};

#endif
