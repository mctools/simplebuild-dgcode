#include "Core/Types.hh"//must include first to build on all platforms
#include "G4DataCollect/G4DataCollect.hh"
#include "DCSteppingAction.hh"
#include "DCEventAction.hh"

#include "G4RunManager.hh"//FIXME
#include <stdexcept>

#include "G4Utils/STUserActionInitHelper.hh"

namespace G4DataCollectInternals
{
  struct DB {
    std::mutex mutex;
    DCSteppingAction * s_stepact = nullptr;
    DCEventAction * s_evtact = nullptr;
    std::function<G4UserSteppingAction*()> m_extraactionfct_stepping;
    std::function<G4UserEventAction*()> m_extraactionfct_event;
    G4Interfaces::StepFilterBase* stepfilter = nullptr;
    G4Interfaces::StepFilterBase* stepkillfilter = nullptr;
    std::vector<std::pair<std::string,std::string>> metadata;
  };
  DB& getDB() {
    static DB db;
    return db;
  }
}

void G4DataCollect::installHooks(const char* outputFile, const char* mode)
{
  std::map<std::string, GriffFormat::Format::MODE> modemap;
  modemap["FULL"] = GriffFormat::Format::MODE_FULL;
  modemap["REDUCED"] = GriffFormat::Format::MODE_REDUCED;
  modemap["MINIMAL"] = GriffFormat::Format::MODE_MINIMAL;
  if (modemap.find(mode)==modemap.end()) {
    printf("G4DataCollect::installHooks ERROR: mode flag must"
           " be one of \"FULL\", \"RECUCED\" or \"MINIMAL\". It was instead \"%s\"\n",mode);
    assert(false);
    return;
  }

  //For efficiency we use a class derived from G4UserSteppingAction as the
  //book-keeping class. We use a helper G4UserEventAction to provide an
  //EndOfEventAction hook as well.

  static std::atomic<unsigned> ncalled = {0};
  if ( ncalled.fetch_add(1) > 0 )
    throw std::logic_error("G4DataCollect::installHooks called twice in succession");

  auto& db = G4DataCollectInternals::getDB();
  std::lock_guard<std::mutex>(db.mutex);

  namespace UAIH = G4Utils::UserActionInitHelper;
  // UAIH::addUserSteppingAction
  // UAIH::getUserSteppingAction

    //  G4RunManager * rm = G4RunManager::GetRunManager();

  std::string outputFile_str(outputFile);
  auto themode = modemap[mode];


  UAIH::addWrappingUserSteppingActionFct([outputFile_str,
                                          themode]
                                         ( G4UserSteppingAction * existing )
                                         -> G4UserSteppingAction *
  {
    auto& db2 = G4DataCollectInternals::getDB();
    std::lock_guard<std::mutex>(db2.mutex);

    if ( db2.m_extraactionfct_stepping && existing )
      throw std::runtime_error("Too many sources of stepping actions");
    if ( db2.m_extraactionfct_stepping && !existing )
      existing = db2.m_extraactionfct_stepping();
    db2.m_extraactionfct_stepping = nullptr;

    db2.s_stepact = new G4DataCollectInternals::DCSteppingAction(outputFile_str.c_str(),
                                                                 themode,
                                                                 existing);
    if ( db2.stepfilter )
      db2.s_stepact->setStepFilter(db2.stepfilter);
    if ( db2.stepkillfilter )
      db2.s_stepact->setStepKillFilter(db2.stepkillfilter);
    for ( auto& e : db2.metadata )
      db2.s_stepact->setMetaData( e.first, e.second );
    db2.stepfilter = nullptr;
    db2.stepkillfilter = nullptr;
    db2.metadata.clear();
    return db2.s_stepact;
  });

  UAIH::addWrappingUserEventActionFct([]( G4UserEventAction * existing )
                                      -> G4UserEventAction *
  {
    auto& db2 = G4DataCollectInternals::getDB();
    std::lock_guard<std::mutex>(db2.mutex);
    if (!db2.s_stepact)
      throw std::runtime_error("G4DataCollect event action needs stepping"
                               " action to be created first");

    if ( db2.m_extraactionfct_event && existing )
      throw std::runtime_error("Too many sources of event actions");
    if ( db2.m_extraactionfct_event && !existing )
      existing = db2.m_extraactionfct_event();
    db2.m_extraactionfct_event = nullptr;

    db2.s_evtact = new G4DataCollectInternals::DCEventAction(db2.s_stepact,
                                                             existing);
    return db2.s_evtact;
  });

  // //  G4UserSteppingAction * existingStepAct = UAIH::getUserSteppingAction();//const_cast<G4UserSteppingAction*>(rm->GetUserSteppingAction());
  // G4UserEventAction * existingEventAct = UAIH::getUserEventAction();// const_cast<G4UserEventAction*>(rm->GetUserEventAction());
  // // G4UserSteppingAction * existingStepAct = const_cast<G4UserSteppingAction*>(rm->GetUserSteppingAction());
  // // G4UserEventAction * existingEventAct = const_cast<G4UserEventAction*>(rm->GetUserEventAction());

  // //  G4DataCollectInternals::s_stepact = new G4DataCollectInternals::DCSteppingAction(outputFile,modemap[mode],existingStepAct);
  // G4DataCollectInternals::s_evtact = new G4DataCollectInternals::DCEventAction(G4DataCollectInternals::s_stepact,existingEventAct);



  // UAIH::addUserEventAction(G4DataCollectInternals::s_evtact);

  // rm->SetUserAction(G4DataCollectInternals::s_stepact);
  // rm->SetUserAction(G4DataCollectInternals::s_evtact);
  UAIH::ensureRegisterWithRunManager();
}

void G4DataCollect::installUserSteppingAction(std::function<G4UserSteppingAction*()>ua)
{
  if (!ua)
    throw std::runtime_error("G4DataCollect::installUserSteppingAction"
                             " called with null function");
  auto& db = G4DataCollectInternals::getDB();
  std::lock_guard<std::mutex>(db.mutex);
  static std::atomic<unsigned> ncalled = {0};
  if ( ncalled.fetch_add(1) > 0 )
    throw std::runtime_error("G4DataCollect::installUserSteppingAction"
                             " called twice");
  if ( db.s_stepact ) {
    //Already installed, just fire!
    db.s_stepact->setOtherAction( ua() );
  } else {
    //delay:
    db.m_extraactionfct_stepping = std::move(ua);
  }
}

void G4DataCollect::installUserEventAction(std::function<G4UserEventAction*()>ua)
{
  if (!ua)
    throw std::runtime_error("G4DataCollect::installUserEventAction"
                             " called with null function");
  auto& db = G4DataCollectInternals::getDB();
  std::lock_guard<std::mutex>(db.mutex);
  static std::atomic<unsigned> ncalled = {0};
  if ( ncalled.fetch_add(1) > 0 )
    throw std::runtime_error("G4DataCollect::installUserEventAction"
                             " called twice");
  if ( db.s_evtact ) {
    //Already installed, just fire!
    db.s_evtact->setOtherAction( ua() );
  } else {
    //delay:
    db.m_extraactionfct_event = std::move(ua);
  }
}

// void G4DataCollect::installUserEventAction(std::function<G4UserEventAction*()>ua)
// {
//   assert(ua);
//   assert(G4DataCollectInternals::s_evtact&&"installHooks not called before installUserSteppingAction");
//   G4DataCollectInternals::s_evtact->setOtherAction(ua);
// }

void G4DataCollect::setStepFilter(G4Interfaces::StepFilterBase*sf)
{
  static std::atomic<unsigned> ncalled = {0};
  if ( ncalled.fetch_add(1) > 0 )
    throw std::runtime_error("G4DataCollect::setStepFilter called twice");
  auto& db = G4DataCollectInternals::getDB();
  std::lock_guard<std::mutex>(db.mutex);
  if ( db.s_stepact )
    db.s_stepact->setStepFilter(sf);
  else
    db.stepfilter = sf;
}

void G4DataCollect::setStepKillFilter(G4Interfaces::StepFilterBase*sf)
{
  static std::atomic<unsigned> ncalled = {0};
  if ( ncalled.fetch_add(1) > 0 )
    throw std::runtime_error("G4DataCollect::setStepKillFilter called twice");
  auto& db = G4DataCollectInternals::getDB();
  std::lock_guard<std::mutex>(db.mutex);
  if ( db.s_stepact )
    db.s_stepact->setStepKillFilter(sf);
  else
    db.stepkillfilter = sf;
}

void G4DataCollect::finish()
{
  //FIXME
  //  G4RunManager * rm = G4RunManager::GetRunManager();
  auto& db = G4DataCollectInternals::getDB();
  std::lock_guard<std::mutex>(db.mutex);

  if (db.s_evtact)
    {
      db.s_evtact->shutdown();
      db.s_evtact = nullptr;
      // G4UserEventAction * existingEvtAct = db.s_evtact->otherAction();
      // rm->SetUserAction(existingEvtAct);
      // delete db.s_evtact;
    }
  if (db.s_stepact)
    {
      db.s_stepact->shutdown();
      db.s_stepact = nullptr;
      // G4UserSteppingAction * existingStepAct = db.s_stepact->otherAction();
      // rm->SetUserAction(existingStepAct);
      // delete db.s_stepact;//This also closes any open output file
    }
}

void G4DataCollect::setMetaData(const std::string& key,const std::string& value)
{
  auto& db = G4DataCollectInternals::getDB();
  std::lock_guard<std::mutex>(db.mutex);
  //  assert(G4DataCollectInternals::s_stepact&&"installHooks not called before setMetaData");
  if (key.size()>0&&key[0]=='^')
    throw std::runtime_error("G4DataCollect::setMetaData ERROR: Meta Data"
                             " keys can not start with '^'\n");
  if (db.s_stepact) {
    db.s_stepact->setMetaData(key,value);
  } else {
    db.metadata.emplace_back(key, value);
  }
}

void G4DataCollect::setUserData(const std::string& key,const std::string& value)
{
  auto& db = G4DataCollectInternals::getDB();
  std::lock_guard<std::mutex>(db.mutex);
  //  assert(G4DataCollectInternals::s_stepact&&"installHooks not called before setUserData");
  std::string tmp;
  tmp.clear();
  tmp+="^";
  tmp+=key;
  if (db.s_stepact) {
    db.s_stepact->setMetaData(tmp,value);
  } else {
    db.metadata.emplace_back(tmp, value);
  }
}

