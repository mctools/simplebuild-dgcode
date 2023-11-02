#include "Core/Python.hh"
#include "G4Launcher/Launcher.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Interfaces/StepFilterBase.hh"
#include "G4Interfaces/PhysListProviderBase.hh"
#include "G4RunManager.hh"
#include "G4UserSteppingAction.hh"
#include "G4UserEventAction.hh"
#include "G4Version.hh"

namespace G4Launcher_py {

  namespace {
    class PyObjResourceGuard : public G4Launcher::Launcher::ResourceGuard {
    public:
      PyObjResourceGuard( py::object o ) : m_o(o) {}
    private:
      py::object m_o;
    };
  }

  void Launcher_addResourceGuard( G4Launcher::Launcher& l, py::object o )
  {
    l.addResourceGuard( std::make_shared<PyObjResourceGuard>( o ) );
  }

  std::string Launcher_rndEvtMsgMode(G4Launcher::Launcher& l)
  {
    return l.rndEvtMsgMode();
  }

  G4ThreeVector pytuple2g4vect(const py::tuple&t)
  {
    if ( py::len(t) != 3 )
      throw std::runtime_error("pytuple2g4vect got tuple of invalid length.");
    assert(py::len(t)==3);

    double x,y,z;
    try {
      x = t[0].cast<double>();
      y = t[1].cast<double>();
      z = t[2].cast<double>();
    } catch ( py::cast_error& ) {
      throw std::runtime_error("pytuple2g4vect got tuple of invalid content (must be floats).");
    }
    return G4ThreeVector(x,y,z);
  }

  void Launcher_addPreGenHook( G4Launcher::Launcher& launcher, G4Interfaces::PreGenCallBack * cb )
  {
    assert( cb != nullptr );
    std::shared_ptr<G4Interfaces::PreGenCallBack> shptr_cb = cb->shared_from_this();
    launcher.addPreGenHook(shptr_cb);
  }
  void Launcher_addPostGenHook( G4Launcher::Launcher& launcher, G4Interfaces::PostGenCallBack * cb )
  {
    assert( cb != nullptr );
    std::shared_ptr<G4Interfaces::PostGenCallBack> shptr_cb = cb->shared_from_this();
    launcher.addPostGenHook(shptr_cb);
  }

  void Launcher_setGeo( G4Launcher::Launcher& launcher, py::object o )
  {
    if ( !py::isinstance<G4Interfaces::GeoConstructBase>( o ) )
      throw std::runtime_error("Ojbect of invalid type passed to .setGeo(..)");
    launcher.setGeo(py::cast<G4Interfaces::GeoConstructBase*>( o ));
    Launcher_addResourceGuard( launcher, o );
  }

  void Launcher_setGen( G4Launcher::Launcher& launcher, py::object o )
  {
    if ( !py::isinstance<G4Interfaces::ParticleGenBase>( o ) )
      throw std::runtime_error("Ojbect of invalid type passed to .setGen(..)");
    launcher.setGen(py::cast<G4Interfaces::ParticleGenBase*>( o ));
    Launcher_addResourceGuard( launcher, o );
  }

  void Launcher_setFilter( G4Launcher::Launcher& launcher, py::object o )
  {
    if ( !py::isinstance<G4Interfaces::StepFilterBase>( o ) )
      throw std::runtime_error("Ojbect of invalid type passed to .setFilter(..)");
    launcher.setFilter(py::cast<G4Interfaces::StepFilterBase*>( o ));
    Launcher_addResourceGuard( launcher, o );
  }


  void Launcher_setKillFilter( G4Launcher::Launcher& launcher, py::object o )
  {
    if ( !py::isinstance<G4Interfaces::StepFilterBase>( o ) )
      throw std::runtime_error("Ojbect of invalid type passed to .setKillFilter(..)");
    launcher.setKillFilter(py::cast<G4Interfaces::StepFilterBase*>( o ));
    Launcher_addResourceGuard( launcher, o );
  }


  G4Launcher::Launcher* Launcher_pyinit( py::list args )
  {
    G4Interfaces::GeoConstructBase* geo(nullptr);
    G4Interfaces::ParticleGenBase* gen(nullptr);
    G4Interfaces::StepFilterBase* filter(nullptr);
    for( auto& e : args ) {
      if ( py::isinstance<G4Interfaces::GeoConstructBase>( e ) ) {
        if ( geo )
          throw std::runtime_error("Too many geometry objects passed to Launcher.");
        geo = py::cast<G4Interfaces::GeoConstructBase*>( e );
        continue;
      }
      if ( py::isinstance<G4Interfaces::ParticleGenBase>( e ) ) {
        if ( gen )
          throw std::runtime_error("Too many particle generator objects passed to Launcher.");
        gen = py::cast<G4Interfaces::ParticleGenBase*>( e );
        continue;
      }
      if ( py::isinstance<G4Interfaces::StepFilterBase>( e ) ) {
        if ( filter )
          throw std::runtime_error("Too many step filter objects passed to Launcher.");
        filter = py::cast<G4Interfaces::StepFilterBase*>( e );
        continue;
      }
      //error:
      std::ostringstream ss;
      ss<<"Unknown argument for Launcher: \""<< py::cast<std::string>(py::str(e))<<"\"";
      throw std::runtime_error(ss.str());
    }
    return new G4Launcher::Launcher( geo, gen, filter );//Returning new object, due to singleton nature of G4Launcher.
  }

  void Launcher_setParticleGun1(G4Launcher::Launcher& l,int pdgcode, double eKin,
                                const py::tuple& pos,const py::tuple& momdir)
  {
    l.setParticleGun(pdgcode,eKin,pytuple2g4vect(pos),pytuple2g4vect(momdir));
  }
  void Launcher_setParticleGun2(G4Launcher::Launcher& l,const char* particleName, double eKin,
                                const py::tuple& pos,const py::tuple& momdir)
  {
    l.setParticleGun(particleName,eKin,pytuple2g4vect(pos),pytuple2g4vect(momdir));
  }

  void Launcher_setOutput_1arg(G4Launcher::Launcher& l,const char* filename) { l.setOutput(filename); }
  void Launcher_setVis_0args(G4Launcher::Launcher& l) { l.setVis(); }
  void Launcher_startSession_0args(G4Launcher::Launcher& l) { l.startSession(); }

  namespace {
    class HookWrapper : public G4Launcher::Launcher::HookFct {
    public:
      HookWrapper(py::object o) : m_o(o) {}
    virtual ~HookWrapper() {}
      virtual void operator()() { m_o(); }
    private:
      py::object m_o;
    };
  }
  void HookFct_addPrePreInitHook(G4Launcher::Launcher&l,py::object f) { l.addPrePreInitHook(std::make_shared<HookWrapper>(f)); }
  void HookFct_addPreInitHook(G4Launcher::Launcher&l,py::object f) { l.addPreInitHook(std::make_shared<HookWrapper>(f)); }
  void HookFct_addPostInitHook(G4Launcher::Launcher&l,py::object f) { l.addPostInitHook(std::make_shared<HookWrapper>(f)); }
  void HookFct_addPostSimHook(G4Launcher::Launcher&l,py::object f) { l.addPostSimHook(std::make_shared<HookWrapper>(f)); }
  void HookFct_addPostMPHook(G4Launcher::Launcher&l,py::object f) { l.addPostMPHook(std::make_shared<HookWrapper>(f)); }
  int g4version() { return G4VERSION_NUMBER; }
  const char* g4versionstr() { return G4Version.c_str(); }

  class LauncherExtraPars : public Utils::ParametersBase {
  public:
    //To be called in the constructor of derived classes, defining which parameters are available:
    void pyaddPD(const std::string&n, double defval) { addParameterDouble(n,defval); }
    void pyaddPI(const std::string&n, int defval) { addParameterInt(n,defval); }
    void pyaddPB(const std::string&n, bool defval) { addParameterBoolean(n,defval); }
    void pyaddPS(const std::string&n, const std::string& defval) { addParameterString(n,defval); }
  };
}

PYTHON_MODULE( mod )
{
  pyextra::pyimport("G4Interfaces");
  pyextra::pyimport("Utils.ParametersBase");

  //We could expose G4RunManager, G4UserSteppingAction and G4UserEventAction as
  //well (at least register_ptr so it can be passed around on the python side)
  //... update: can't find register_ptr anymore, so here is a very basic
  //declaration of at least the stepping and event actions:
  py::class_<G4UserSteppingAction>(mod,"G4UserSteppingAction")
    ;
  py::class_<G4UserEventAction>(mod,"G4UserEventAction")
    ;
  mod.def("g4version",G4Launcher_py::g4version);
  mod.def("g4versionstr",G4Launcher_py::g4versionstr);
  py::class_<G4Launcher_py::LauncherExtraPars,Utils::ParametersBase>(mod,"LauncherExtraPars")
    .def(py::init<>())
    .def("addParameterBoolean",&G4Launcher_py::LauncherExtraPars::pyaddPB)
    .def("addParameterInt",&G4Launcher_py::LauncherExtraPars::pyaddPI)
    .def("addParameterString",&G4Launcher_py::LauncherExtraPars::pyaddPS)
    .def("addParameterDouble",&G4Launcher_py::LauncherExtraPars::pyaddPD)
    ;

  py::class_<G4Launcher::Launcher>(mod,"Launcher",py::dynamic_attr())//py::dynamic_attr to allow py-layer to add attributes (in _launcher.py)
    .def(py::init(&G4Launcher_py::Launcher_pyinit),py::return_value_policy::reference)
    .def("setGeo",&G4Launcher_py::Launcher_setGeo)
    .def("setGen",&G4Launcher_py::Launcher_setGen)
    .def("setFilter",&G4Launcher_py::Launcher_setFilter)
    .def("setKillFilter",&G4Launcher_py::Launcher_setKillFilter)
    .def("setMultiProcessing",&G4Launcher::Launcher::setMultiProcessing)
    .def("getRunManager",&G4Launcher::Launcher::getRunManager,py::return_value_policy::reference)
    .def("setVis",&G4Launcher::Launcher::setVis)
    .def("setVis",&G4Launcher_py::Launcher_setVis_0args)
    .def("startSession",&G4Launcher::Launcher::startSession)
    .def("startSession",&G4Launcher_py::Launcher_startSession_0args)
    .def("startSimulation",&G4Launcher::Launcher::startSimulation)
    .def("setRndEvtMsgMode",&G4Launcher::Launcher::setRndEvtMsgMode)
    .def("rndEvtMsgMode",&G4Launcher_py::Launcher_rndEvtMsgMode)
    .def("setPhysicsList",&G4Launcher::Launcher::setPhysicsList)
    .def("setPhysicsListProvider",&G4Launcher::Launcher::setPhysicsListProvider)
    .def("hasPhysicsListProvider",&G4Launcher::Launcher::hasPhysicsListProvider)
    .def("setParticleGun",&G4Launcher_py::Launcher_setParticleGun1)
    .def("setParticleGun",&G4Launcher_py::Launcher_setParticleGun2)
    .def("setOutput",&G4Launcher::Launcher::setOutput)
    .def("setOutput",&G4Launcher_py::Launcher_setOutput_1arg)
    .def("closeOutput",&G4Launcher::Launcher::closeOutput)
    .def("noRandomSetup",&G4Launcher::Launcher::noRandomSetup)
    .def("setSeed",&G4Launcher::Launcher::setSeed)
    .def("setUserSteppingAction",&G4Launcher::Launcher::setUserSteppingAction)
    .def("setUserEventAction",&G4Launcher::Launcher::setUserEventAction)
    .def("cmd",&G4Launcher::Launcher::cmd)
    .def("cmd_preinit",&G4Launcher::Launcher::cmd_preinit)
    .def("cmd_postinit",&G4Launcher::Launcher::cmd_postinit)
    .def("init",&G4Launcher::Launcher::init)
    .def("initVis",&G4Launcher::Launcher::initVis)
    .def("getMultiProcessing",&G4Launcher::Launcher::getMultiProcessing)
    .def("GetNoRandomSetup",&G4Launcher::Launcher::GetNoRandomSetup)
    .def("getSeed",&G4Launcher::Launcher::getSeed)
    .def("getOutputFile",&G4Launcher::Launcher::getOutputFile)
    .def("getOutputMode",&G4Launcher::Launcher::getOutputMode)
    .def("getVis",&G4Launcher::Launcher::getVis)
    .def("getGeo",&G4Launcher::Launcher::getGeo,py::return_value_policy::reference)
    .def("getGen",&G4Launcher::Launcher::getGen,py::return_value_policy::reference)
    .def("getFilter",&G4Launcher::Launcher::getFilter,py::return_value_policy::reference)
    .def("getPhysicsList",&G4Launcher::Launcher::getPhysicsList)
    .def("getPrintPrefix",&G4Launcher::Launcher::getPrintPrefix)
    .def("allowMultipleSettings",&G4Launcher::Launcher::allowMultipleSettings)
    .def("addHist",&G4Launcher::Launcher::addHist)
    .def("setUserData",&G4Launcher::Launcher::setUserData)
    .def("allowFPE",&G4Launcher::Launcher::allowFPE)
    .def("dumpGDML",&G4Launcher::Launcher::dumpGDML)
    .def("preInitDone",&G4Launcher::Launcher::preInitDone)
    .def("rmIsInit",&G4Launcher::Launcher::rmIsInit)
    .def("addPrePreInitHook",&G4Launcher_py::HookFct_addPrePreInitHook)
    .def("addPreInitHook",&G4Launcher_py::HookFct_addPreInitHook)
    .def("addPostInitHook",&G4Launcher_py::HookFct_addPostInitHook)
    .def("addPostSimHook",&G4Launcher_py::HookFct_addPostSimHook)
    .def("addPostMPHook",&G4Launcher_py::HookFct_addPostMPHook)
    .def("prepreinit_hook",&G4Launcher_py::HookFct_addPrePreInitHook)
    .def("preinit_hook",&G4Launcher_py::HookFct_addPreInitHook)
    .def("postinit_hook",&G4Launcher_py::HookFct_addPostInitHook)
    .def("postsim_hook",&G4Launcher_py::HookFct_addPostSimHook)
    .def("postmp_hook",&G4Launcher_py::HookFct_addPostMPHook)
    .def("addPreGenHook",&G4Launcher_py::Launcher_addPreGenHook)
    .def("addPostGenHook",&G4Launcher_py::Launcher_addPostGenHook)
    .def("_addResourceGuard",&G4Launcher_py::Launcher_addResourceGuard)
    .def("_shutdown",&G4Launcher::Launcher::shutdown)
    ;

  mod.def("getTheLauncher",&G4Launcher::Launcher::getTheLauncher,py::return_value_policy::reference);
}
