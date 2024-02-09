#ifndef G4Launcher_Launcher_hh
#define G4Launcher_Launcher_hh

#include "Core/Types.hh"
#include <memory>

// This Launcher class wraps G4RunManager and takes care of most of the setup of
// a Geant4 job. At the end you should call either startSimulation or
// startSession. For interactive visualisation, call setVis(..) followed by
// startSession(..).
//
// Required step before calling initXXX(..) or startXXX():
//
// * You MUST either register a geometry with setGeo or by using getRunManager()
//   and registering a G4VUserDetectorConstruction.
//
// Required step before calling initXXX(..) or startSimulation():
//
// * Register a generator with setGen(..)
//
// Optional steps before calling initXXX(..) or startXXX():
//
// * If you wish a different physics list than QGSP_BIC_HP_EMZ you must call
//   setPhysicsList(..) (or optionally use getRunManager() and register a custom
//   physicsList *before* calling init(..)). Set the physics list to "none" to
//   avoid the framework registering a physics list at all. Alternatively,
//   provide an instance of PhysListProviderBase (it will be owned by the
//   launcher).
//
// * If you do not call setOutput(..) before init, it will default to
//   output in output.griff. If you wish no GRIFF output, call setOutput("none")
//   before init(..).
//
// * Be sure to register any custom user actions with the run manager before
//   init(..).
//
// * call noRandomSetup() if you wish to prevent the Launcher from messing with
//   random streams at all (otherwise it will setup the Xoroshiro128+ generator
//   (via NCrystal) with the chosen seed and will ensure seeds are printed
//   etc.).
//
// * Call setSeed(..) to set the random seed.

#include "G4ThreeVector.hh"//todo remove?

class G4RunManager;
class G4UserSteppingAction;
class G4UserEventAction;

namespace G4Interfaces {
  class GeoConstructBase;
  class ParticleGenBase;
  class PreGenCallBack;
  class PostGenCallBack;
  class StepFilterBase;
  class PhysListProviderBase;
}

namespace G4Launcher {

  class Launcher {
  public:

    static Launcher * getTheLauncher();

    Launcher(G4Interfaces::GeoConstructBase* geo = nullptr,
             G4Interfaces::ParticleGenBase* gen = nullptr,
             G4Interfaces::StepFilterBase* filter = nullptr);//supply geo/gen/filter here or with setVis/setGeo/setFilter calls
    ~Launcher();

    //Geometry must be set before beam-on either by the next method or by
    //registering a geometry directly with the run manager:
    void setGeo(G4Interfaces::GeoConstructBase*);//Recommended

    //Set the generator:
    void setGen(G4Interfaces::ParticleGenBase*);

    //Optionally set the filter for which steps are written to Griff files:
    void setFilter(G4Interfaces::StepFilterBase*);

    //A different stronger filter which actually also kills steps to prevent further simulations:
    void setKillFilter(G4Interfaces::StepFilterBase*);

    //Optionally pick a physics list. Default is "QGSP_BIC_HP_EMZ" (except "QGSP_BIC_HP" is default for Geant4 versions older than v10.3)
    void setPhysicsList(const char *);//For picking a G4 reference list
    void setPhysicsListProvider(G4Interfaces::PhysListProviderBase*);//Named callback to create phys list
    bool hasPhysicsListProvider() const;

    //For custom modifications:
    G4RunManager * getRunManager();

    //Schedules visualisation manager.
    void setVis(const char* visengine = "OGL");

    //Will setup remaining items as required and init visualisation (if setVis
    //was called) and the runManager, and then drop the user to an interactive
    //Geant4 prompt. All setXXX(..) methods must have been invoked before this:
    void startSession();

    //Will call init() if not done already then and call beamOn(nevents) on the
    //run manager. Set nevents=0 to mean "until source runs out":
    void startSimulation(unsigned nevents);

    //convenience methods which will call setGen(..) with a SingleParticleGun instance.
    void setParticleGun(int pdgcode, double eKin, const G4ThreeVector& pos,const G4ThreeVector& momdir);
    void setParticleGun(const char* particleName, double eKin, const G4ThreeVector& pos,const G4ThreeVector& momdir);
    //void setParticleGun(int pdgcode, const G4ThreeVector& pos,const G4ThreeVector& momdir);

    //Target GRIFF file. Use "none" to disable GRIFF output (mode must be FULL, REDUCED or MINIMAL):
    void setOutput(const char* filename, const char * mode = "FULL");
    void closeOutput();//Hook for expert users to close the Griff file early.

    void noRandomSetup();
    void setSeed(std::uint64_t seed);

    //Tune how often the Random Manager prints out the event number and seed
    //(mode must be "ALWAYS", "NEVER" or "ADAPTABLE", with the latter being the default):
    void setRndEvtMsgMode(const char * mode);
    const std::string& rndEvtMsgMode() const;//mode or empty if setRndEvtMsgMode never called

    //To avoid conflicts with the GRIFF file hooks, register custom stepping and
    //event actions here rather than with the run-manager. Note that you should
    //only construct your action class instances *after* calling init() on the
    //launcher:
    void setUserSteppingAction(G4UserSteppingAction*);
    void setUserEventAction(G4UserEventAction*);

    //Direct access to Geant4 commands:
    void cmd(const char*);//apply immediately
    void cmd_preinit(const char*);//schedule the command to just before runmgr init
    void cmd_postinit(const char*);//schedule the command to just after runmgr init

    //For interactive sessions, append entries to the command history (accessible by "ArrowUp" on the keyboard):
    void addHist(const char*);

    //Will setup remaining items as required and call runManager Initialize().
    //All setXXX(..) methods must have been invoked before this:
    void init();

    void initVis();//preinits visualisation after setVis(..) command (in case
                   //you want to be able to specify "/vis/..." commands before
                   //calling startSession(). Also initializes runmanager.

    bool preInitDone() const;//true if preinit has been carried out
    bool rmIsInit() const;//true if run manager has been initialised

    //Preliminary support for multi-process execution. Only use when both seed
    //and output are controlled by the standards above. Note that the final
    //result after calling startSimulation(nevts) will be nprocs outputfiles,
    //each with nevts evts inside:
    void setMultiProcessing(unsigned nprocs);

    //Register custom user data to be embedded in the output griff file:
    void setUserData(const char* key, const char* value);

    void allowFPE();//disable FPE checking. This is obviously not a good idea to do!

    void dumpGDML(const char* filename);

    //getters:
    unsigned getMultiProcessing() const;
    bool GetNoRandomSetup() const;
    std::uint64_t getSeed() const;
    const char* getOutputFile() const;
    const char* getOutputMode() const;
    const char* getVis() const;
    const char* getPhysicsList() const;
    G4Interfaces::GeoConstructBase* getGeo() const;
    G4Interfaces::ParticleGenBase* getGen() const;
    G4Interfaces::StepFilterBase* getFilter() const;
    G4Interfaces::StepFilterBase* getKillFilter() const;
    //This is just for the python wrapper:
    const char* getPrintPrefix() const;
    void allowMultipleSettings();

    //Add callbacks allowing a high degree of customisation (Launcher will
    //assume ownership of the registered hookfct objects).

    class HookFct {
    public:
      virtual ~HookFct(){};
      virtual void operator()() = 0;
    };
    using HookFctPtr = std::shared_ptr<HookFct>;

    void addPrePreInitHook(HookFctPtr);
    void addPreInitHook(HookFctPtr);
    void addPostInitHook(HookFctPtr);
    //hook called after simulation is done (will be called in all process in
    //case of multiprocessing):
    void addPostSimHook(HookFctPtr);
    //hook called in parent process only, when multiprocessing only, and only
    //after all children finished successfully (can be used to merge output
    //files):
    void addPostMPHook(HookFctPtr);
    //Hooks to be installed on the generator:
    void addPreGenHook(std::shared_ptr<G4Interfaces::PreGenCallBack>);
    void addPostGenHook(std::shared_ptr<G4Interfaces::PostGenCallBack>);

    //Internal method for well-defined shut-down order, independent of python's
    //garbage collection. It releases all internal objects and deletes the run
    //manager. This method is automatically called after launcher.go() in python
    //scripts, or else in the destructor. No other methods should be called
    //after a shutdown.
    void shutdown();

    //Internal method for advanced lifetime synchronisation, register shared
    //pointer which will be cleared upon shutdown();
    class ResourceGuard {
    public:
      virtual ~ResourceGuard() = default;
    };
    void addResourceGuard( std::shared_ptr<ResourceGuard> );

    //Forbid both moves and copies (so getTheLauncher() will always return the same object:
    Launcher( const Launcher& ) = delete;
    Launcher& operator=( const Launcher& ) = delete;
    Launcher( Launcher&& ) = delete;
    Launcher& operator=( Launcher&& ) = delete;

  private:
    struct Imp;
    Imp * m_imp = nullptr;
  };
}

#endif
