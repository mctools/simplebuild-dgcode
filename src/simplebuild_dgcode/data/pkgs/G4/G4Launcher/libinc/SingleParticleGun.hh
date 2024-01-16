#ifndef G4Launcher_SingleParticleGun_hh
#define G4Launcher_SingleParticleGun_hh

#include "G4Interfaces/ParticleGenBase.hh"

class G4ParticleGun;

namespace G4Launcher {

  struct SingleParticleGun : public G4Interfaces::ParticleGenBase
  {
    SingleParticleGun() : ParticleGenBase("G4Launcher/SingleParticleGun"), m_gun(0)
    {
      addParameterDouble("x_meter",0,-1.0e5,1.0e5);
      addParameterDouble("y_meter",0,-1.0e5,1.0e5);
      addParameterDouble("z_meter",0,-1.0e5,1.0e5);
      addParameterDouble("energy_eV",100,0.0,10.0e12);
      addParameterDouble("momdirx",0);
      addParameterDouble("momdiry",0);
      addParameterDouble("momdirz",1);
      addParameterInt("nParticles",1,1,100000);

      //The user must call exactly one of these methods:
      addParameterString("particleName","");
      addParameterInt("pdgCode",0);
    }

    virtual ~SingleParticleGun();
    void init();
    void gen(G4Event*);

  protected:
    bool validateParameters();

  private:
    G4ParticleGun * m_gun;
  };

}

#endif
