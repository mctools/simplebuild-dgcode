#ifndef G4StdGenerators_FlexGen_hh
#define G4StdGenerators_FlexGen_hh

#include "G4Interfaces/ParticleGenBase.hh"

//Generator which shoots a number (fixed or poisson distributed) of particles of
//a given type. The parameters (position, direction, energy) of the particles
//can be specified as either a fixed value or a range in which they are picked
//randomly and uniformly.

class FlexGen : public G4Interfaces::ParticleGenBase {
public:
  FlexGen();
  virtual ~FlexGen();
  void init();
  void gen(G4Event*);

protected:
  bool validateParameters();

private:
  struct Imp;
  Imp * m_imp;
};


#endif
