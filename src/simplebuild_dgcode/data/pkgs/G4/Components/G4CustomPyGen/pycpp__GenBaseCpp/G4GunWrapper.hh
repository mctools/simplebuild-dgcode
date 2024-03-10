#ifndef G4CustomPyGen_pycpp_G4GunWrapper_hh
#define G4CustomPyGen_pycpp_G4GunWrapper_hh

#include "Core/Types.hh"
#include "G4Utils/GenUtils.hh"
#include "G4ParticleGun.hh"
#include "G4Event.hh"
#include <cassert>

namespace G4CustomPyGen {

  class G4GunWrapper {
  public:
    G4GunWrapper();
    virtual ~G4GunWrapper();

    /////////////////////////////////////////////////////////////////
    // Methods available to people implementing python generators: //
    /////////////////////////////////////////////////////////////////

    void set_type(const char * particle_name);
    void set_type(int pdg_code);
    void set_energy(double);
    void set_wavelength(double);
    void set_wavelength_angstrom(double);
    void set_position(double,double,double);
    void set_direction(double,double,double);
    void set_random_direction();
    void set_momentum(double,double,double);
    void set_weight(double);
    void set_time(double);

    //Fire the gun (only needed when generating more than 1 particle per event
    //or allow_empty_events() has been called):
    void fire();

    //Call to prevent gun auto-firing once when the user didn't (needed if
    //events can be empty):
    void allow_empty_events();

    ////////////////////////
    // For the framework: //
    ////////////////////////

    void setEvent(G4Event*e) { m_currentEvt = e; m_nfired = 0; }
    void fireIfNeeded();//fires the gun if not done already this event

    bool hasFired() const { return m_nfired>0; }
  protected:
    G4ParticleGun * m_gun;
    G4Event * m_currentEvt;
    double m_weight;
    unsigned m_nfired;
    bool m_empty_allowed;
    G4int m_lastpdg;
  };
}

#include "G4GunWrapper.icc"

#endif
