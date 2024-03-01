#ifndef G4LAUNCHER_ENABLE_GDML_DUMP
// We could also look at HAS_Geant4_GDML, but for now be cautious.
#  define G4LAUNCHER_ENABLE_GDML_DUMP 0
#endif

#include "Core/Types.hh"//must include first to build on all platforms
#include "Core/String.hh"
#include "G4Launcher/Launcher.hh"
#include "G4Launcher/SingleParticleGun.hh"
#include "G4PhysicsLists/PhysListMgr.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Interfaces/StepFilterBase.hh"
#include "G4Interfaces/PhysListProviderBase.hh"
#include "G4DataCollect/G4DataCollect.hh"
#include "G4Random/RandomManager.hh"
#include "G4Interfaces/FrameworkGlobals.hh"
#include "G4NCrystalRel/G4NCInstall.hh"
#include "G4NCrystalRel/G4NCManager.hh"
#include "Core/FPE.hh"
#include "Units/Units.hh"
#include "MultiProcessingMgr.hh"
#include "G4Utils/Flush.hh"
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4ParticleGun.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4VisExecutive.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4VModularPhysicsList.hh"
#include "G4OpticalPhysics.hh"
#include "Utils/Format.hh"
#include "Utils/ByteStream.hh"
#include "G4VUserPhysicsList.hh"
#if G4LAUNCHER_ENABLE_GDML_DUMP
#include "G4GDMLParser.hh"
#endif
#include <limits>
#include <stdexcept>
#include "launcher_impl_ts.hh"

#include "NCrystal/internal/NCString.hh"

struct G4Launcher::Launcher::Imp {
  static Launcher *& singleTonLauncherPtr()
  {
    static Launcher * ptr = nullptr;
    return ptr;
  }

  Imp()
    : m_rm(0),
      m_vis(0),
      m_geo(0),
      m_gen(0),
      m_filter(0),
      m_killfilter(0),
      m_outputmode("FULL"),
      m_isinit_pre(false),
      m_isinit_vis_pre(false),
      m_isinit_rm(false),
      m_postmphooks_alreadyfired(false),
      m_norandom(false),
      m_seed(0),
      m_nprocs(0),
      m_allowMultipleSettings(false),
      m_physicsListProvider(0),
      m_dofpe(true),
      m_closedGriff(false)
  {
    if (std::string(FrameworkGlobals::printPrefix()).empty())
      FrameworkGlobals::setPrintPrefix("G4Launcher:: ");
  }
  ~Imp(){ closeGriff(); delete m_physicsListProvider; delete m_vis; delete m_rm; }
  void ensureCreateRM() {
    if (!m_rm) {
      //this->print("Creating G4RunManager");
      m_rm = new G4RunManager;
      m_rm->SetVerboseLevel(0);
    }
  }

  G4RunManager * m_rm;
  G4VisExecutive * m_vis;
  G4Interfaces::GeoConstructBase * m_geo;
  G4Interfaces::ParticleGenBase * m_gen;
  G4Interfaces::StepFilterBase * m_filter;
  G4Interfaces::StepFilterBase * m_killfilter;

  //griff output:
  std::string m_output;
  std::string m_outputmode;
  //Visualisation:
  std::string m_visengine;

  bool m_isinit_pre;
  bool m_isinit_vis_pre;
  bool m_isinit_rm;

  std::string m_gdmlfile;
  std::vector<std::string> m_cmds_preinit;
  std::vector<std::string> m_cmds_postinit;
  std::vector<std::string> m_cmdhist;
  std::vector<std::string> m_cmdlog;//for recording cmds in GRIFF output
  std::vector<std::pair<std::string,std::string> > m_userdata;

  std::vector<HookFctPtr> m_prepreinithooks;
  std::vector<HookFctPtr> m_preinithooks;
  std::vector<HookFctPtr> m_postinithooks;
  std::vector<HookFctPtr> m_postsimhooks;
  std::vector<HookFctPtr> m_postmphooks;
  bool m_postmphooks_alreadyfired;
  std::vector<std::shared_ptr<G4Interfaces::PostGenCallBack>> m_postgenhooks;
  std::vector<std::shared_ptr<G4Interfaces::PreGenCallBack>> m_pregenhooks;

  std::vector<std::shared_ptr<ResourceGuard>> m_resourceGuards;


  //random:
  bool m_norandom;
  std::uint64_t m_seed;
  std::string m_rnd_evtmsg_mode;

  //mp:
  unsigned m_nprocs;

  bool m_allowMultipleSettings;
  std::string m_physicsListName;
  G4Interfaces::PhysListProviderBase * m_physicsListProvider;
  static const char * defaultPhysicsListName()
  {
#if G4VERSION_NUMBER >= 1030
    return "QGSP_BIC_HP_EMZ";
#else
    return "QGSP_BIC_HP";
#endif
  }
  bool m_dofpe;
  bool m_closedGriff;

  void closeGriff() { if (m_output!="none" && !m_closedGriff ) { G4DataCollect::finish(); m_closedGriff=true; } }

  const char * prefix() { return FrameworkGlobals::printPrefix(); }

  void error(const char * e)
  {
    G4Utils::flush();
    printf("\n%sERROR: %s!\n\n",prefix(),e);
    std::cout.flush();
    throw std::runtime_error(e);
    exit(1);
  }

  void print(const char * msg)
  {
    G4Utils::flush();
    std::cout.flush();
    printf("%s%s\n",prefix(),msg);
    std::cout.flush();
  }

  void preinit();
  void preinit_vis(Launcher *);
  void finalMetadata();//just before launch, to catch the last user cmds
};

//G4Launcher::Launcher * G4Launcher::Launcher::Imp::s_theLauncher = nullptr;

G4Launcher::Launcher * G4Launcher::Launcher::getTheLauncher()
{
  return Imp::singleTonLauncherPtr();
}

G4Launcher::Launcher::Launcher(G4Interfaces::GeoConstructBase*geo,
                               G4Interfaces::ParticleGenBase* gen,
                               G4Interfaces::StepFilterBase*filter)
  : m_imp(new Imp)
{
  if (Imp::singleTonLauncherPtr()) {
    m_imp->print("ERROR: Multiple instances of Launcher are not allowed!");
    exit(1);
  }
  Imp::singleTonLauncherPtr() = this;

  if (geo)
    setGeo(geo);
  if (gen)
    setGen(gen);
  if (filter)
    setFilter(filter);
}

void G4Launcher::Launcher::shutdown()
{
  if (!m_imp)
    return;
  delete m_imp;
  m_imp = 0;
}

G4Launcher::Launcher::~Launcher()
{
  shutdown();
}

void G4Launcher::Launcher::setMultiProcessing(unsigned nprocs)
{
  if (m_imp->m_isinit_pre)
    m_imp->error("setMultiProcessing called too late");
  if (m_imp->m_nprocs!=0&&!m_imp->m_allowMultipleSettings)
    m_imp->error("attempt to call setMultiProcessing twice");
  if (!nprocs)
    m_imp->error("argument to setMultiProcessing should be non-zero");
  if (nprocs==1&&!m_imp->m_allowMultipleSettings)
    m_imp->print("WARNING: Calling setMultiProcessing(1) is not multi-processing at all!");
  m_imp->m_nprocs = nprocs;
}

void G4Launcher::Launcher::setGeo(G4Interfaces::GeoConstructBase* geo)
{
  if (m_imp->m_isinit_pre)
    m_imp->error("setGeo called too late");
  if (!geo)
    m_imp->error("setGeo called with NULL pointer");
  assert(geo);
  if (m_imp->m_geo)
    m_imp->error("attempt to call setGeo twice");
  m_imp->m_geo = geo;
}

void G4Launcher::Launcher::setGen(G4Interfaces::ParticleGenBase* gen)
{
  if (m_imp->m_isinit_pre)
    m_imp->error("setGen called too late");
  if (!gen)
    m_imp->error("setGen called with NULL pointer");
  assert(gen);
  if (m_imp->m_gen)
    m_imp->error("attempt to call setGen twice");
  m_imp->m_gen = gen;
}

void G4Launcher::Launcher::setFilter(G4Interfaces::StepFilterBase*f)
{
  if (m_imp->m_isinit_pre)
    m_imp->error("setFilter called too late");
  if (!f)
    m_imp->error("setFilter called with NULL pointer");
  assert(f);
  if (m_imp->m_filter)
    m_imp->error("attempt to call setFilter twice");
  m_imp->m_filter = f;
}

void G4Launcher::Launcher::setKillFilter(G4Interfaces::StepFilterBase*f)
{
  if (m_imp->m_isinit_pre)
    m_imp->error("setKillFilter called too late");
  if (!f)
    m_imp->error("setKillFilter called with NULL pointer");
  assert(f);
  if (m_imp->m_killfilter)
    m_imp->error("attempt to call setKillFilter twice");
  m_imp->m_killfilter = f;
}

void G4Launcher::Launcher::setOutput(const char* filename,const char * mode)
{
  if (m_imp->m_isinit_pre)
    m_imp->error("setOutput called too late");
  if (!m_imp->m_output.empty()&&!m_imp->m_allowMultipleSettings)
    m_imp->error("attempt to call setOutput twice");
  m_imp->m_output = filename;
  if (m_imp->m_output.empty())
    m_imp->error("empty filename provided to setOutput");
  m_imp->m_outputmode=mode;
  if (m_imp->m_outputmode!="FULL"&&m_imp->m_outputmode!="REDUCED"&&m_imp->m_outputmode!="MINIMAL")
    m_imp->error("setOutput called with invalid mode. Must be FULL, REDUCED or MINIMAL");
}

void G4Launcher::Launcher::closeOutput()
{
  assert(m_imp);
  m_imp->closeGriff();
}

void G4Launcher::Launcher::setVis(const char* visengine)
{
  if (m_imp->m_isinit_pre||m_imp->m_isinit_vis_pre)
    m_imp->error("setVis called too late");
  if (!m_imp->m_visengine.empty()&&!m_imp->m_allowMultipleSettings)
    m_imp->error("attempt to call setVis twice");
  m_imp->m_visengine = visengine;
  if (m_imp->m_visengine.empty()&&!m_imp->m_allowMultipleSettings)
    m_imp->error("empty visengine provided to setVis");
}

void G4Launcher::Launcher::Imp::preinit_vis(Launcher * launcher)
{
  preinit();
  launcher->init();
  if (m_isinit_vis_pre)
    return;//silent return, might not be an error
  m_isinit_vis_pre = true;
  if (m_visengine.empty())
    return;
  print("Initialising visualisation manager");
  m_vis = new G4VisExecutive;
  m_vis->Initialize();
  launcher->cmd("/vis/scene/create");
  std::string tmp = "/vis/open ";
  tmp += m_visengine;
  launcher->cmd( tmp.c_str() );
  launcher->cmd("/vis/modeling/trajectories/create/drawByCharge colcharge");
  launcher->cmd("/vis/modeling/trajectories/create/drawByParticleID colpdg");
  launcher->cmd("/vis/modeling/trajectories/colpdg/set gamma yellow");
  launcher->cmd("/vis/modeling/trajectories/colpdg/set neutron blue");
  launcher->cmd("/vis/modeling/trajectories/colpdg/set electron red");
  launcher->cmd("/vis/modeling/trajectories/colpdg/set alpha green");
  launcher->cmd("/vis/modeling/trajectories/colpdg/set \"Li7[0.0]\" white");
  launcher->cmd("/vis/viewer/set/upVector 0 1 0");//NB: upvector and viewpoint direction must not be parallel
  launcher->cmd("/vis/viewer/set/viewpointThetaPhi 60 -170");
  launcher->cmd("/vis/viewer/set/lineSegmentsPerCircle 200");
  launcher->cmd("/vis/drawVolume");//basically this means "zoom to world volume"
  launcher->cmd("/vis/scene/add/trajectories");
  launcher->cmd("/vis/scene/add/hits");
  launcher->cmd("/vis/scene/endOfEventAction accumulate 0");
  const char * datadir_c = getenv("SBLD_DATA_DIR");
  std::string datadir(datadir_c?datadir_c:"");
  if (datadir.empty()) {
    print("WARNING: $SBLD_DATA_DIR environment variable not set or is empty!");
    launcher->cmd("/control/macroPath .");
  } else {
    std::string tmp2;
    Utils::string_format(tmp2,"/control/macroPath .:%s",datadir.c_str());
    launcher->cmd(tmp2.c_str());
  }

}

bool G4Launcher::Launcher::preInitDone() const
{
  return m_imp->m_isinit_pre;
}

void G4Launcher::Launcher::Imp::preinit()
{

  //setup various missing parts
  if (m_isinit_pre)
    return;//silent return, might not be an error
  m_isinit_pre = true;

  for ( auto& e : m_prepreinithooks )
    (*e)();

  print("Pre-init started");

  if (m_dofpe) {
    Core::catch_fpe();
    print("Installing FPE detection");
  } else {
    print("WARNING: FPE detection explicitly disabled!");
  }

  if (m_output.empty())
    m_output = "output.griff";

  if (m_nprocs>1) {
    printf("%sMulti-processing requested with %i processes\n",prefix(),m_nprocs);
    if (!m_gen)
      error("A particle generator must be registered with setGen(..) for multiprocessing to work");
    G4Launcher::MultiProcessingMgr::scheduleMP(m_gen,m_nprocs);
  }

  //Random engine

  if (m_norandom) {
    if (m_seed)
      error("Calling both noRandomSetup() and setSeed() is consistent");
    if (!m_rnd_evtmsg_mode.empty())
      error("Calling both noRandomSetup() and setRndEvtMsgMode() is consistent");
    print("Will not set up random engine");
  } else {
    if (!m_seed) {
      m_seed = 123456789;
      printf("%ssetSeed() not called, picking standard seed.\n",prefix());
      std::cout.flush();
    }
    auto evtmsgmode = RandomManager::EVTMSG_ADAPTABLE;
    if (m_rnd_evtmsg_mode=="ALWAYS") {
      evtmsgmode = RandomManager::EVTMSG_ALWAYS;
    } else if (m_rnd_evtmsg_mode=="NEVER") {
      evtmsgmode = RandomManager::EVTMSG_NEVER;
    } else { assert(m_rnd_evtmsg_mode.empty()||m_rnd_evtmsg_mode=="ADAPTABLE"); }
    RandomManager::init(m_seed, evtmsgmode );
  }

  ensureCreateRM();

  //Physics list
  if (!m_rm->GetUserPhysicsList()) {
    if (m_physicsListProvider && m_physicsListProvider->getName() != m_physicsListName) {
      error("Inconsistency detected in physics list provider setup");
    }
    if (m_physicsListName.empty())
      m_physicsListName = defaultPhysicsListName();
    if (m_physicsListName!="none") {
      if (m_physicsListProvider)
        m_physicsListName = m_physicsListProvider->getName();
      printf("%sSetting up physics list %s\n",prefix(),m_physicsListName.c_str());
      G4VUserPhysicsList * pl(0);
      if (m_physicsListProvider) {
        pl = m_physicsListProvider->construct();
        if (!pl)
          error("Problems creating physics list via provider");
      } else {
        std::vector<std::string> pln_parts;
        Core::split(pln_parts, m_physicsListName,"+");
        bool extra_optical(false), extra_ts(false);
        while (pln_parts.size()>1) {
          if (pln_parts.back()=="TS") {
            extra_ts = true;
            pln_parts.pop_back();
          } else if (pln_parts.back()=="OPTICAL") {
            extra_optical = true;
            pln_parts.pop_back();
          } else {
            throw std::runtime_error(std::string("Invalid physics list name: ")+m_physicsListName);
          }
        }

        pl = PhysListMgr::createList( pln_parts.front() );
        if (!pl) {
          printf("%sUnknown physics list: %s\n",prefix(),pln_parts.front().c_str());
          error("Physics list name is not known");
        }
        if (extra_optical||extra_ts) {
          G4VModularPhysicsList * plm = dynamic_cast<G4VModularPhysicsList*>(pl);
          if (!plm)
            throw std::runtime_error("Can't use +TS/+OPTICAL with non-modular physics list");
          if (extra_optical) {
            plm->RegisterPhysics(new G4OpticalPhysics());
          }
          if (extra_ts) {
            G4Launcher_impl_ts::registerTSPhysics(plm);
          }
        }
      }
      m_rm->SetUserInitialization(pl);


    } else {
      printf("%sRequested to not set up physics list (presumably done in user hooks)\n",prefix());
    }
  }

  if (m_rm->GetUserDetectorConstruction()) {
    if (m_geo)
      error("GeoConstructBase was provided via setGeo(..) but a G4VUserDetectorConstruction was also set directly on the run manager");
    print("Found geometry already registered with the run manager.");
  } else {
    if (!m_geo) {
      error("No geometry was provided");
    } else {
      print("Setting up geometry:");
      m_geo->dump((std::string(Imp::prefix())+"  --> ").c_str());
      m_rm->SetUserInitialization(m_geo);
    }
  }

  if (m_rm->GetUserPrimaryGeneratorAction()) {
    error("A G4VUserPrimaryGeneratorAction was set directly on the run manager, but"
          " this framework only accepts ParticleGenBase implementations");
  } else {
    if (!m_gen) {
      //Not an error unless startSimulation is called (because use might simply wish to visualise the geometry)
    } else {
      print("Setting up particle generation:");
      m_gen->dump((std::string(Imp::prefix())+"  --> ").c_str());
      RandomManager::attach(m_gen);
      m_rm->SetUserAction(m_gen->getAction());
    }
  }

  if (m_rm->GetUserSteppingAction())
    error("User stepping action installed by hand on the run-manager. Please install via the launcher instead.");
  if (m_rm->GetUserEventAction())
    error("User event action installed by hand on the run-manager. Please install via the launcher instead.");

  if (m_output=="none") {
    print("Requested to not set output of GRIFF files:");
    if (m_filter||m_killfilter)
      error("Filter registered but GRIFF file output disabled.");
  } else {
    printf("%sInstalling hooks for capturing%s output in \"%s\" in mode %s\n",
           Imp::prefix(),(m_filter||m_killfilter?" filtered":""),m_output.c_str(),m_outputmode.c_str());
    if (m_killfilter) {
      print("GRIFF output applies a kill-filter:");
      m_killfilter->dump((std::string(Imp::prefix())+"  --> ").c_str());
    }
    if (m_filter) {
      print("GRIFF output is filtered by:");
      m_filter->dump((std::string(Imp::prefix())+"  --> ").c_str());
    }
    std::cout.flush();
    G4DataCollect::installHooks(m_output.c_str(),m_outputmode.c_str());
  }

  print("Pre-init done");
}

//hack to access protected method StoreHistory:
struct G4Launcher_Launcher_G4UItcsh : public G4UItcsh {
  void PublicStoreHistory(G4String aCommand) { StoreHistory(aCommand); }
};

void G4Launcher::Launcher::startSession()
{
  m_imp->preinit();
  init();
  m_imp->preinit_vis(this);
  m_imp->finalMetadata();
  cmd("/tracking/verbose 1");
  if (!m_imp->m_visengine.empty())
    cmd("/vis/enable");
  m_imp->print("");
  m_imp->print("**********************************************************");
  m_imp->print("**    Welcome to the Geant4 interactive command line    **");
  m_imp->print("**  Type \"help\" for more info about available commands  **");
  m_imp->print("**********************************************************");
  m_imp->print("");
  m_imp->print("Often used commands:");
  m_imp->print("");
  if (!m_imp->m_isinit_rm) {
    m_imp->print("  /run/initialize [initialise run manager]");
    assert(m_imp->m_cmds_preinit.empty());//TODO: generally throw exceptions rather than assert
    assert(m_imp->m_cmds_postinit.empty());
  }
  if (m_imp->m_visengine.empty())
    m_imp->print("  /run/beamOn N [simulate N events]");
  else
    m_imp->print("  /run/beamOn N [simulate N events and transfer control to graphics window]");
  m_imp->print("  /control/execute MyPackage/mymacro.mac [run mymacro.mac from MyPackage/data]");
  m_imp->print("  /control/execute mymacro.mac [run mymacro.mac from current directory]");
  m_imp->print("");
#if G4VERSION_NUMBER >= 1000
  m_imp->print("  /geometry/test/run [test for geometry clashes]");
#else
  m_imp->print("  /geometry/test/recursive_test [test for geometry clashes]");
  m_imp->print("  /geometry/test/cylinder_test [test for geometry clashes]");
  m_imp->print("  /geometry/test/grid_test [test for geometry clashes]");
#endif
  m_imp->print("  /geometry/test/tolerance 0.000001 mm [change tolerance]");

  if (!m_imp->m_visengine.empty()) {
    m_imp->print("");
    m_imp->print("  /vis/viewer/update [transfer control to graphics window]");
    m_imp->print("  /vis/scene/add/axes x y z l [Add axes at (x,y,z) of length l in colours RGB=XYZ]");
    m_imp->print("  /vis/modeling/trajectories/select colcharge [colour by particle charge]");
    m_imp->print("  /vis/modeling/trajectories/select colpdg [colour by particle type]");
    m_imp->print("  /vis/viewer/save <filename> [create new macro file with cmds to recreate current view]");
    m_imp->print("");
    m_imp->print("Note that if (depending on visengine) control transfers to the graphics window, you");
    m_imp->print("can transfer it back to the cmd line by selecting \"Exit to G4Vis>\" from a menu.");
  }
  m_imp->print("");

  if (!m_imp->m_rm->GetUserPrimaryGeneratorAction()) {
    m_imp->print("");
    m_imp->print("WARNING: No particle generator has been setup. Any attempts to call /run/beamOn will fail!");
    m_imp->print("         => use /vis/viewer/update instead to transfer control to the graphics window.");
    m_imp->print("");
  }
  auto shell = new G4UItcsh;
  for (auto it=m_imp->m_cmdhist.begin();it!=m_imp->m_cmdhist.end();++it) {
    static_cast<G4Launcher_Launcher_G4UItcsh*>(shell)->PublicStoreHistory(it->c_str());
  }
  if (!m_imp->m_cmdhist.empty()) {
    printf("%sNote that %i command(s) have been added to your command history (arrow-up).\n",
           m_imp->prefix(),unsigned(m_imp->m_cmdhist.size()));
    m_imp->print("");
  }

  G4UIterminal(shell).SessionStart();
}

void G4Launcher::Launcher::Imp::finalMetadata()
{
  if (m_output=="none")
    return;
  if (m_filter) {
    //install griff step filter:
    G4DataCollect::setStepFilter(m_filter);
    //record meta-data
    G4DataCollect::setMetaData("filterName",m_filter->getName());
    char * dataS;
    unsigned lengthS;
    std::string tmpS;
    m_filter->serialiseParameters(dataS,lengthS);
    tmpS.assign(dataS,lengthS);//careful with null chars
    G4DataCollect::setMetaData("`filterSerialised",tmpS);//The leading '`' hints that this is binary data
    delete[] dataS;
  }
  if (m_killfilter) {
    //install griff step kill-filter:
    G4DataCollect::setStepKillFilter(m_killfilter);
    //record meta-data
    G4DataCollect::setMetaData("killFilterName",m_killfilter->getName());
    char * dataS;
    unsigned lengthS;
    std::string tmpS;
    m_killfilter->serialiseParameters(dataS,lengthS);
    tmpS.assign(dataS,lengthS);//careful with null chars
    G4DataCollect::setMetaData("`killFilterSerialised",tmpS);//The leading '`' hints that this is binary data
    delete[] dataS;
  }
  if (m_geo) {
    G4DataCollect::setMetaData("geoName",m_geo->getName());
    char * dataS;
    unsigned lengthS;
    std::string tmpS;
    m_geo->serialiseParameters(dataS,lengthS);
    tmpS.assign(dataS,lengthS);//careful with null chars
    G4DataCollect::setMetaData("`geoSerialised",tmpS);//The leading '`' hints that this is binary data
    delete[] dataS;
  }
  if (m_gen) {
    G4DataCollect::setMetaData("genName",m_gen->getName());
    char * dataS;
    unsigned lengthS;
    std::string tmpS;
    m_gen->serialiseParameters(dataS,lengthS);
    tmpS.assign(dataS,lengthS);//careful with null chars
    G4DataCollect::setMetaData("`genSerialised",tmpS);//The leading '`' hints that this is binary data
    delete[] dataS;
  }
  //Record commands:
  unsigned lengthCmdStr = ByteStream::nbytesToWrite(m_cmdlog);
  std::string tmpCmdStr;
  tmpCmdStr.resize(lengthCmdStr);
  char * tmpCmdStrData = const_cast<char*>(tmpCmdStr.c_str());
  ByteStream::write(tmpCmdStrData,m_cmdlog);
  G4DataCollect::setMetaData("`g4Cmds",tmpCmdStr);//The leading '`' hints that this is binary data
  //phys list
  G4DataCollect::setMetaData("G4PhysicsList",m_physicsListName);
  //metadata - custom:
  if (!m_userdata.empty()) {
    auto itE=m_userdata.end();
    for (auto it=m_userdata.begin();it!=itE;++it) {
      printf("%sEmbedding user data: \"%s\" -> \"%s\"\n",prefix(),it->first.c_str(),it->second.c_str());
      G4DataCollect::setUserData(it->first.c_str(),it->second.c_str());
    }
  }
}

void G4Launcher::Launcher::initVis()
{
  m_imp->preinit_vis(this);
}

bool G4Launcher::Launcher::rmIsInit() const
{
  return m_imp->m_isinit_rm;
}

void G4Launcher::Launcher::init()
{
  if (m_imp->m_isinit_rm)
    return;
  m_imp->m_isinit_rm=true;
  m_imp->preinit();

  for ( auto& e : m_imp->m_preinithooks )
    (*e)();

  m_imp->ensureCreateRM();
  {
    auto itE = m_imp->m_cmds_preinit.end();
    for (auto it = m_imp->m_cmds_preinit.begin();it!=itE;++it)
      cmd(it->c_str());
    m_imp->m_cmds_preinit.clear();
  }
  m_imp->print("Calling G4RunManager::Initialize()");
  m_imp->m_rm->Initialize();
  m_imp->print("G4RunManager::Initialize() done");

  {
    auto itE = m_imp->m_cmds_postinit.end();
    for (auto it = m_imp->m_cmds_postinit.begin();it!=itE;++it)
      cmd(it->c_str());
    m_imp->m_cmds_postinit.clear();
  }

  if (!m_imp->m_gdmlfile.empty()) {
#if G4LAUNCHER_ENABLE_GDML_DUMP
    G4GDMLParser gdmlparser;
    gdmlparser.Write(m_imp->m_gdmlfile.c_str(),(const G4VPhysicalVolume*)0,false);
#else
  m_imp->print("WARNING: GDML dumping not enabled in Launcher.cc!!");
#endif
  }

  //Install NCrystal if needed:
  if (m_imp->m_physicsListName!="PL_Empty")
    G4NCrystalRel::installOnDemand();

  for (auto& e : m_imp->m_postinithooks )
    (*e)();

}

void G4Launcher::Launcher::startSimulation(unsigned nevents)
{
  if (!m_imp->m_visengine.empty())
    m_imp->error("Do not use startSimulation after setVis. Use startSession instead.");
  m_imp->ensureCreateRM();
  if (!m_imp->m_isinit_rm)
    init();
  if (!m_imp->m_gen)
    m_imp->error("No particle generation was set up before startSimulation(..) was called.!");
  assert(m_imp->m_rm->GetUserPrimaryGeneratorAction());

  m_imp->finalMetadata();
  if (nevents==0) {
    if (m_imp->m_gen->unlimited())
      m_imp->error("startSimulation called with nevents=0 despite particle generator"
                   " being able to supply an unlimited number of events. Aborting job"
                   " which might never finish.");
    nevents = std::numeric_limits<G4int>::max();
    printf("%sStarting simulation\n",m_imp->prefix());
  } else {
    printf("%sStarting simulation of %i events\n",m_imp->prefix(),nevents);
  }
  std::cout.flush();

  for (auto it = m_imp->m_pregenhooks.begin(); it!=m_imp->m_pregenhooks.end(); ++it)
    m_imp->m_gen->installPreGenCallBack(*it);
  m_imp->m_pregenhooks.clear();
  for (auto it = m_imp->m_postgenhooks.begin(); it!=m_imp->m_postgenhooks.end(); ++it)
    m_imp->m_gen->installPostGenCallBack(*it);
  m_imp->m_postgenhooks.clear();

  m_imp->m_rm->BeamOn(nevents);
  if (m_imp->m_gen->reachedLimit()) {
    assert(!m_imp->m_gen->unlimited());
    printf("%sNo more events to process.\n",FrameworkGlobals::printPrefix());
  }

  m_imp->print("Simulation done");

  for ( auto& e : m_imp->m_postsimhooks )
    (*e)();

  if (FrameworkGlobals::isForked()&&FrameworkGlobals::isParent()) {
    G4Launcher::MultiProcessingMgr::checkAnyChildren(true);
    m_imp->print("Simulation done in all processes");
    //fire post mp hooks (should be safe even if more are added from inside hooks):
    unsigned i = 0;
    while (i<m_imp->m_postmphooks.size()) {
      HookFct* hf = m_imp->m_postmphooks[i++].get();
      (*hf)();
    }
    m_imp->m_postmphooks_alreadyfired = true;
  }
}

void G4Launcher::Launcher::setParticleGun(int pdgcode, double eKin, const G4ThreeVector& pos,const G4ThreeVector& momdir)
{
  if (m_imp->m_isinit_pre)
    m_imp->error("setParticleGun called too late");
  auto gun = new SingleParticleGun;
  gun->setParameterDouble("x_meter",pos.x()/Units::meter);
  gun->setParameterDouble("y_meter",pos.y()/Units::meter);
  gun->setParameterDouble("z_meter",pos.z()/Units::meter);
  gun->setParameterDouble("energy_eV",eKin/Units::eV);
  gun->setParameterDouble("momdirx",momdir.x());
  gun->setParameterDouble("momdiry",momdir.y());
  gun->setParameterDouble("momdirz",momdir.z());
  gun->setParameterInt("nParticles",1);
  gun->setParameterInt("pdgCode",pdgcode);
  setGen(gun);
}


void G4Launcher::Launcher::dumpGDML(const char* filename)
{
  if (m_imp->m_isinit_rm)
    m_imp->error("dumpGDML called too late");
  if (!m_imp->m_gdmlfile.empty())
    m_imp->error("dumpGDML called twice");
  if (!filename)
    m_imp->error("dumpGDML called with null string");
  m_imp->m_gdmlfile=filename;
  if (m_imp->m_gdmlfile.empty())
    m_imp->error("dumpGDML called with empty string");
}

void G4Launcher::Launcher::setParticleGun(const char* particleName, double eKin, const G4ThreeVector& pos,const G4ThreeVector& momdir)
{
  if (m_imp->m_isinit_pre)
    m_imp->error("setParticleGun called too late");
  auto gun = new SingleParticleGun;
  gun->setParameterDouble("x_meter",pos.x()/Units::meter);
  gun->setParameterDouble("y_meter",pos.y()/Units::meter);
  gun->setParameterDouble("z_meter",pos.z()/Units::meter);
  gun->setParameterDouble("energy_eV",eKin/Units::eV);
  gun->setParameterDouble("momdirx",momdir.x());
  gun->setParameterDouble("momdiry",momdir.y());
  gun->setParameterDouble("momdirz",momdir.z());
  gun->setParameterInt("nParticles",1);
  gun->setParameterString("particleName",particleName);
  setGen(gun);
}

void G4Launcher::Launcher::setPhysicsList(const char * plc)
{
  if (m_imp->m_isinit_pre)
    m_imp->error("setPhysicsList called too late");
  if (!m_imp->m_physicsListName.empty()&&!m_imp->m_allowMultipleSettings)
    m_imp->error("attempt to call setPhysicsList twice");
  std::string s_plc = (plc?plc:"");
  //Handle aliases for "PL_Empty":
  if ( s_plc == "empty" )
    s_plc = "PL_Empty";
  //Handle deprecated ESS_ prefix:
  if ( NCrystal::startswith( s_plc, "ESS_" ) ) {
    std::string s_plcnew("PL_");
    s_plcnew += s_plc.substr(4);
    std::ostringstream ss;
    ss << "WARNING: Trying to map deprecated physics list name "<<s_plc
       <<" to " << s_plcnew;
    m_imp->print( ss.str().c_str() );
    s_plc = s_plcnew;
  }
  m_imp->m_physicsListName = s_plc;
  if (m_imp->m_physicsListName.empty()&&!m_imp->m_allowMultipleSettings)
    m_imp->error("empty physics list name provided to setPhysicsList");
  if (m_imp->m_physicsListProvider
      && m_imp->m_physicsListProvider->getName() != m_imp->m_physicsListName) {
    delete m_imp->m_physicsListProvider;
    m_imp->m_physicsListProvider = 0;
  }

}

void G4Launcher::Launcher::setPhysicsListProvider(G4Interfaces::PhysListProviderBase*ppl)
{
  if (!ppl)
    m_imp->error("setPhysicsListProvider called with null instance");
  if (m_imp->m_physicsListName==ppl->getName())
    m_imp->m_physicsListName.clear();
  setPhysicsList(ppl->getName());//This call takes care of error handling
  m_imp->m_physicsListProvider = ppl;
}

bool G4Launcher::Launcher::hasPhysicsListProvider() const
{
  return m_imp->m_physicsListProvider!=0;
}


void G4Launcher::Launcher::cmd_preinit(const char* cmdstr)
{
  if (m_imp->m_isinit_rm)
    m_imp->error("cmd_preinit called after G4RunManager was initialised");
  else
    m_imp->m_cmds_preinit.push_back(cmdstr);//schedule for invocation just before runmgr init
}

void G4Launcher::Launcher::cmd_postinit(const char* cmdstr)
{
  if (m_imp->m_isinit_rm)
    cmd(cmdstr);//fire immediately
  else
    m_imp->m_cmds_postinit.push_back(cmdstr);//schedule for post-initialisation
}

void G4Launcher::Launcher::cmd(const char* cmdstr)
{
  G4Utils::flush();
  if (strncmp(getPhysicsList(),"PL_Empty",8)==0&&strncmp(cmdstr,"/process/",9)==0) {
    printf("%sIgnoring command due to PL_Empty physics list: %s\n",m_imp->prefix(),cmdstr);
    return;
  }
  printf("%sApplying command: %s\n",m_imp->prefix(),cmdstr);
  std::cout.flush();
  m_imp->m_cmdlog.push_back(cmdstr);
  //inspired by code in G4Py
  G4UImanager* UImgr= G4UImanager::GetUIpointer();
  G4int returnVal= UImgr->ApplyCommand(cmdstr);
  if( returnVal == fCommandSucceeded ) return;

  G4int paramIndex = returnVal % 100;
  G4int commandStatus = returnVal - paramIndex;

  switch(commandStatus) {
    case fCommandSucceeded:
      break;
    case fCommandNotFound:
      G4cout << "command <" << UImgr-> SolveAlias(cmdstr) << "> not found" << G4endl;
      break;
    case fIllegalApplicationState:
      G4cout << "illegal application state -- command refused" << G4endl;
      break;
    case fParameterOutOfRange:
      G4cout << "Parameter out of range" << G4endl;
      break;
    case fParameterOutOfCandidates:
      G4cout << "Parameter is out of candidate list (index " << paramIndex << ")" << G4endl;
      break;
    case fParameterUnreadable:
      G4cout << "Parameter is wrong type and/or is not omittable (index " << paramIndex << ")" << G4endl;
      break;
    case fAliasNotFound:
      break;
    default:
      G4cout << "command refused (" << commandStatus << ")" << G4endl;
      break;
  }
  G4Utils::flush();
  m_imp->error("command failed");
  return;
}

G4RunManager * G4Launcher::Launcher::getRunManager()
{
  m_imp->ensureCreateRM();
  return m_imp->m_rm;
}

void G4Launcher::Launcher::noRandomSetup()
{
  m_imp->m_norandom = true;
}

const std::string& G4Launcher::Launcher::rndEvtMsgMode() const
{
  return m_imp->m_rnd_evtmsg_mode;
}

void G4Launcher::Launcher::setRndEvtMsgMode(const char * mode)
{
  if (m_imp->m_isinit_pre)
    m_imp->error("setRndEvtMsgMode called too late");
  if (!mode)
    m_imp->error("setRndEvtMsgMode called with null string");
  auto& smode = m_imp->m_rnd_evtmsg_mode;
  if (!smode.empty())
    m_imp->error("setRndEvtMsgMode called twice");
  smode = mode;
  if ( smode!="ALWAYS" && smode!="NEVER" && smode!="ADAPTABLE" )
    m_imp->error("setRndEvtMsgMode called with invalid mode. Must be ALWAY, NEVER or ADAPTABLE");
}


void G4Launcher::Launcher::setSeed(std::uint64_t seed)
{
  if (!seed)
    m_imp->error("Only call setSeed with non-zero argument.");
  m_imp->m_seed = seed;
}

void G4Launcher::Launcher::setUserSteppingAction(G4UserSteppingAction*ua)
{
  if (!m_imp->m_isinit_rm)
    m_imp->error("setUserSteppingAction must only be called after init()");
  if (!ua)
    m_imp->error("Only call setUserSteppingAction with non-zero argument.");
  if (m_imp->m_output!="none")
    G4DataCollect::installUserSteppingAction(ua);//via Griff
  else
    m_imp->m_rm->SetUserAction(ua);//directly on the run manager
}

void G4Launcher::Launcher::setUserEventAction(G4UserEventAction*ua)
{
  if (!m_imp->m_isinit_rm)
    m_imp->error("setUserEventAction must only be called after init()");
  if (!ua)
    m_imp->error("Only call setUserEventAction with non-zero argument.");
  if (m_imp->m_output!="none")
    G4DataCollect::installUserEventAction(ua);//via Griff
  else
    m_imp->m_rm->SetUserAction(ua);//directly on the run manager
}

unsigned G4Launcher::Launcher::getMultiProcessing() const
{
  return m_imp->m_nprocs;
}

bool G4Launcher::Launcher::GetNoRandomSetup() const
{
  return m_imp->m_norandom;
}

std::uint64_t G4Launcher::Launcher::getSeed() const
{
  return m_imp->m_seed;
}

const char* G4Launcher::Launcher::getOutputFile() const
{
  return m_imp->m_output.c_str();
}

const char* G4Launcher::Launcher::getOutputMode() const
{
  return m_imp->m_outputmode.c_str();
}

const char* G4Launcher::Launcher::getVis() const
{
  return m_imp->m_visengine.c_str();
}

const char* G4Launcher::Launcher::getPhysicsList() const
{
  if (m_imp->m_physicsListProvider)
    return m_imp->m_physicsListProvider->getName();
  return m_imp->m_physicsListName.empty() ? m_imp->defaultPhysicsListName() : m_imp->m_physicsListName.c_str();
}

G4Interfaces::GeoConstructBase* G4Launcher::Launcher::getGeo() const
{
  return m_imp->m_geo;
}

G4Interfaces::ParticleGenBase* G4Launcher::Launcher::getGen() const
{
  return m_imp->m_gen;
}

G4Interfaces::StepFilterBase* G4Launcher::Launcher::getFilter() const
{
  return m_imp->m_filter;
}

G4Interfaces::StepFilterBase* G4Launcher::Launcher::getKillFilter() const
{
  return m_imp->m_killfilter;
}

const char* G4Launcher::Launcher::getPrintPrefix() const
{
  return m_imp->prefix();
}

void G4Launcher::Launcher::allowMultipleSettings()
{
  m_imp->m_allowMultipleSettings = true;
}

void G4Launcher::Launcher::addHist(const char*c)
{
  m_imp->m_cmdhist.push_back(c);
}

void G4Launcher::Launcher::setUserData(const char* key, const char* value)
{
  m_imp->m_userdata.push_back(std::pair<std::string,std::string>(key,value));
}

void G4Launcher::Launcher::allowFPE()
{
  m_imp->m_dofpe = false;
}

void G4Launcher::Launcher::addPrePreInitHook(HookFctPtr hf) { m_imp->m_prepreinithooks.push_back(std::move(hf)); }
void G4Launcher::Launcher::addPreInitHook(HookFctPtr hf) { m_imp->m_preinithooks.push_back(std::move(hf)); }
void G4Launcher::Launcher::addPostInitHook(HookFctPtr hf) { m_imp->m_postinithooks.push_back(std::move(hf)); }
void G4Launcher::Launcher::addPostSimHook(HookFctPtr hf) { m_imp->m_postsimhooks.push_back(std::move(hf)); }
void G4Launcher::Launcher::addPreGenHook(std::shared_ptr<G4Interfaces::PreGenCallBack> hf) { m_imp->m_pregenhooks.push_back(std::move(hf)); }
void G4Launcher::Launcher::addPostGenHook(std::shared_ptr<G4Interfaces::PostGenCallBack> hf) { m_imp->m_postgenhooks.push_back(std::move(hf)); }

void G4Launcher::Launcher::addPostMPHook(HookFctPtr hf) {
  if (m_imp->m_postmphooks_alreadyfired)
    (*hf)();
  else {
    m_imp->m_postmphooks.push_back(std::move(hf));
  }
}

void G4Launcher::Launcher::addResourceGuard( std::shared_ptr<ResourceGuard> rg )
{
  assert(m_imp);
  m_imp->m_resourceGuards.push_back(std::move(rg));
}
