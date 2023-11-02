#ifndef G4GriffGen_GriffGen_hh
#define G4GriffGen_GriffGen_hh

#include "G4Interfaces/ParticleGenBase.hh"
#include <map>

class GriffDataReader;
class G4ParticleGun;
class G4ParticleDefinition;
namespace GriffDataRead { class Step; }

class GriffGen : public G4Interfaces::ParticleGenBase {
public:
  GriffGen();
  virtual ~GriffGen();
  virtual void init();
  virtual void gen(G4Event*);
  virtual bool unlimited() const { return false; }
protected:
  void initDR();
  virtual bool validateParameters();
  std::map<int,G4ParticleDefinition*> m_partdefs;
  GriffDataReader * m_dr;
  G4ParticleGun * m_gun;
  int m_nprocs;
  bool m_primary_only;
  void shootPreStep(G4Event*evt, const GriffDataRead::Step*);
};

#endif
