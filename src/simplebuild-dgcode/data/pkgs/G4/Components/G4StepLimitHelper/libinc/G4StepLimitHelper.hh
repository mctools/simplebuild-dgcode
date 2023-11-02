#ifndef G4StepLimitHelper_h
#define G4StepLimitHelper_h 1
#include "globals.hh"
#include <vector>

class G4StepLimitHelper
{
public:
  G4StepLimitHelper();
  ~G4StepLimitHelper();
  void setLimit();
  void addLimit(G4int pgdCode,
      const char* volName,
      G4double uStepMax );
  void setWorldLimit();
  void addWorldLimit(G4int pgdCode,
      G4double uStepMax );

protected:
  template <typename volume>
  void set(volume *v, G4int pgdCode, G4double uStepMax);


  std::vector<G4int> m_pgdCode;
  std::vector<G4String> m_volName;
  std::vector<G4double> m_uStepMax;

  std::vector<G4int> m_world_pgdCode;
  std::vector<G4double> m_world_uStepMax;

};


#endif























