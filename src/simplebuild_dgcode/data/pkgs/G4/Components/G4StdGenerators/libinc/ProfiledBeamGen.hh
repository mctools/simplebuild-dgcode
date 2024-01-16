#ifndef G4StdGenerators_ProfiledBeamGen_hh
#define G4StdGenerators_ProfiledBeamGen_hh

#include "G4Interfaces/ParticleGenBase.hh"

//Generator which emulates a non-divergent beam travelling in the direction of
//positive z, and starting at z=0 (by default). It can be given a Gaussian
//spread in the xy-plane or a flat distribution shaped either like an ellipsoid
//or a rectangle.
//
//Each event will contain a number (fixed or poisson distributed) of particles
//of a given type. Either the energy of the particle can be specified or, for
//neutrons only, the corresponding wavelength or temperature can. In the case of
//wavelength specification, it is also possible to add a Gaussian spread in
//wavelength and when the temperature is set, a perfect thermal spectrum is
//created.

//Possible future additions:
//  * Allow correlations between X and Y positions
//  * Allow double-gaussians or double thermal spectra
//  * Allow initial central position to be changed from (0,0,0)
//  * Allow to modify beam direction
//  * Allow wavelength/thermal spectra for any particle, not just neutrons.
//  * Warn if relativistic + thermal beam (we used classical formulas)

class ProfiledBeamGen : public G4Interfaces::ParticleGenBase {
public:
  ProfiledBeamGen();
  virtual ~ProfiledBeamGen();
  void init();
  void gen(G4Event*);

protected:
  bool validateParameters();

private:
  struct Imp;
  Imp * m_imp;
};

#endif
