#include "Core/Python.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ios.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include <cassert>
#include <stdexcept>

//If installed, will install a post-generation hook that will duplicate all
//particles as geantinoes. Can be used to test where the primary particles would
//have intersected the geometry, in case there were no interactions.

namespace G4GeantinoInserter {
  class PostGenCB : public G4Interfaces::PostGenCallBack {
  private:
    G4ParticleGun * m_gun = nullptr;
    void delayedInit() {
      m_gun = new G4ParticleGun(1);
      auto particle = G4ParticleTable::GetParticleTable()->FindParticle("geantino");
      if (!particle)
        throw std::runtime_error("G4GeantinoInserter ERROR: Could not find geantino in current physics list");
      m_gun->SetParticleDefinition(particle);
    }
  public:
    PostGenCB() = default;
    virtual ~PostGenCB(){ delete m_gun; }
    virtual void postGen(G4Event*evt)
    {
      assert(evt);
      if (!m_gun)
        delayedInit();

      const G4int nvtx_orig = evt->GetNumberOfPrimaryVertex();
      for (G4int i = 0; i < nvtx_orig; ++i) {
        G4PrimaryVertex* vtx = evt->GetPrimaryVertex(i);
        assert(vtx);
        m_gun->SetParticlePosition(vtx->GetPosition());
        m_gun->SetParticleTime(vtx->GetT0());
        const G4double weight = vtx->GetWeight();
        G4int np = vtx->GetNumberOfParticle();
        for (G4int j = 0; j < np; ++j) {
          G4PrimaryParticle* particle = vtx->GetPrimary(j);
          assert(particle);
          if (!particle->GetKineticEnergy()) {
            G4cout << "G4GeantinoInserter WARNING: Ignoring primary particle without kinetic energy" << G4endl;
            continue;
          }
          m_gun->SetParticleMomentumDirection(particle->GetMomentumDirection());
          m_gun->SetParticleEnergy(particle->GetKineticEnergy());
          //generate single-particle vertex, with specified weight:
          int iv = evt->GetNumberOfPrimaryVertex();
          m_gun->GeneratePrimaryVertex(evt);
          if (weight!=1.0)
            evt->GetPrimaryVertex(iv)->SetWeight(weight);
        }
      }
    }
  };

  void install()
  {
    //Get launcher:
    py::object pylauncher = pyextra::pyimport("G4Launcher").attr("getTheLauncher")();
    if (!pylauncher)
      throw std::runtime_error("G4GeantinoInserter.install called before G4Launcher object was created");
    //Create our call-back and register it:
    pyextra::pyimport("G4Interfaces");
    auto cb = std::make_shared<PostGenCB>();
    std::shared_ptr<G4Interfaces::PostGenCallBack> cb_base = cb;
    py::object pycb = py::cast(cb_base);//,py::return_value_policy::copy);
    pylauncher.attr("addPostGenHook")(pycb);
  }
}

PYTHON_MODULE( mod )
{
  mod.def("install",&G4GeantinoInserter::install);
}
