//We need to extend G4HadronElasticPhysicsHP, but need to workaround a faulty
//usage of the "final" keyword (should have been "override"), as discussed on
//https://bugzilla-geant4.kek.jp/show_bug.cgi?id=2574 . We provide a somewhat
//nasty (but reliable) workaround:

#include "G4Version.hh"

#if G4VERSION_NUMBER >= 1120

// Untested as per November 2023, but on the bug report it was indicated that we
// should be able to use this directly with Geant4 11.2.0 and later:
#  include "G4HadronElasticPhysicsHP.hh"

#else
//The workaround is essentially to replicate the class, but not marking
//ConstructProcess as "final". This work, as there is no ABI difference between
//the two versions (e.g. "final"/"override" have compile-time access control
//effects only, they don't show up in the mangled symbols):
#  include "G4HadronElasticPhysics.hh"
class G4HadronElasticPhysicsHP : public G4HadronElasticPhysics {
public:
  explicit G4HadronElasticPhysicsHP( G4int verbosity = 1 );
  virtual ~G4HadronElasticPhysicsHP();
  void ConstructProcess() override;//<=== this "override" is instead of "final" in G4HadronElasticPhysicsHP.hh !!!!
  G4HadronElasticPhysicsHP( G4HadronElasticPhysicsHP& ) = delete;
  G4HadronElasticPhysicsHP& operator=( const G4HadronElasticPhysicsHP& ) = delete;
};
#endif

//Our custom HadElasHP_TS, extending G4HadronElasticPhysicsHP with thermal
//scattering enabled:

namespace G4Launcher_impl_ts {
  namespace {
    class HadElasHP_TS final : public G4HadronElasticPhysicsHP {
    public:
      HadElasHP_TS( G4int verbosity = 1 ) : G4HadronElasticPhysicsHP(verbosity) {}
      virtual ~HadElasHP_TS() = default;
      void ConstructProcess() override;
    };
  }
}

#include "G4NeutronHPThermalScattering.hh"
#include "G4NeutronHPThermalScatteringData.hh"
#include "G4HadronicParameters.hh"//for verbosity

void G4Launcher_impl_ts::HadElasHP_TS::ConstructProcess()
{
  this->G4HadronElasticPhysicsHP::ConstructProcess();

  auto neutron = G4Neutron::Neutron();
  if (!neutron) {
    G4Exception("G4Launcher::HadElasHP_TS::ConstructProcess","HadElasHP_TS01",
                FatalException, "particle \"neutron\" not available.");
    return;
  }
  G4HadronicProcess* hel = GetElasticProcess( neutron );
  if (!hel) {
    G4Exception("G4Launcher::HadElasHP_TS::ConstructProcess","HadElasHP_TS01",
                FatalException, "No hadronic-elastic process available for neutrons.");
    return;
  }

  //The next two lines represent the actual extension of this class with respect
  //to it's base class:
  hel->RegisterMe(new G4NeutronHPThermalScattering());
  hel->AddDataSet(new G4NeutronHPThermalScatteringData());

  if(G4HadronicParameters::Instance()->GetVerboseLevel() > 1)
    G4cout << "### HadronElasticPhysicsHP+TS is constructed " << G4endl;
}

#include "launcher_impl_ts.hh"
#include "G4VModularPhysicsList.hh"
#include <stdexcept>
#include <cassert>

void G4Launcher_impl_ts::registerTSPhysics(G4VModularPhysicsList* physlist)
{
  assert(physlist);
  G4int verbosity = physlist->GetVerboseLevel();
  //In a perfect world we could simply do:
  // physlist->ReplacePhysics(new G4HadronElasticPhysicsHP_TS(verbosity));
  //But the physics type fields appear to be 0 everywhere... (in G4 v 10.0.p03 - to be rechecked!)
  G4int index = 0;
  const G4VPhysicsConstructor* physconst = 0;
  bool removed=false;
  while ((physconst = physlist->GetPhysics(index++))) {
    if (dynamic_cast<const G4HadronElasticPhysicsHP*>(physconst)) {
      if (removed)
        throw std::runtime_error("TS_Wrapper Error: Did not find exactly one G4HadronElasticPhysicsHP"
                                 " on the base physics list (is it really a HP list?)");
      physlist->RemovePhysics(const_cast<G4VPhysicsConstructor*>(physconst));
      removed = true;
    }
  }
  physlist->RegisterPhysics(new ::G4Launcher_impl_ts::HadElasHP_TS(verbosity));
}
