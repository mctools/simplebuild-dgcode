#include "Core/Types.hh"//must include first to build on all platforms
#include "G4DataCollect/G4DataCollect.hh"
#include "DCSteppingAction.hh"
#include "DCEventAction.hh"

#include "G4RunManager.hh"
#include <stdexcept>

namespace G4DataCollectInternals
{
  static DCSteppingAction * s_stepact = 0;
  static DCEventAction * s_evtact = 0;
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

  if (G4DataCollectInternals::s_stepact || G4DataCollectInternals::s_evtact)
    throw std::logic_error("G4DataCollect::installHooks called twice in succession");

  G4RunManager * rm = G4RunManager::GetRunManager();
  G4UserSteppingAction * existingStepAct = const_cast<G4UserSteppingAction*>(rm->GetUserSteppingAction());
  G4UserEventAction * existingEventAct = const_cast<G4UserEventAction*>(rm->GetUserEventAction());

  G4DataCollectInternals::s_stepact = new G4DataCollectInternals::DCSteppingAction(outputFile,modemap[mode],existingStepAct);
  G4DataCollectInternals::s_evtact = new G4DataCollectInternals::DCEventAction(G4DataCollectInternals::s_stepact,existingEventAct);

  rm->SetUserAction(G4DataCollectInternals::s_stepact);
  rm->SetUserAction(G4DataCollectInternals::s_evtact);
}

void G4DataCollect::installUserSteppingAction(G4UserSteppingAction*ua)
{
  assert(ua);
  assert(G4DataCollectInternals::s_stepact&&"installHooks not called before installUserSteppingAction");
  G4DataCollectInternals::s_stepact->setOtherAction(ua);
}

void G4DataCollect::installUserEventAction(G4UserEventAction*ua)
{
  assert(ua);
  assert(G4DataCollectInternals::s_evtact&&"installHooks not called before installUserSteppingAction");
  G4DataCollectInternals::s_evtact->setOtherAction(ua);
}

void G4DataCollect::setStepFilter(G4Interfaces::StepFilterBase*sf)
{
  assert(sf);
  assert(G4DataCollectInternals::s_stepact&&"installHooks not called before setStepFilter");
  G4DataCollectInternals::s_stepact->setStepFilter(sf);
}

void G4DataCollect::setStepKillFilter(G4Interfaces::StepFilterBase*sf)
{
  assert(sf);
  assert(G4DataCollectInternals::s_stepact&&"installHooks not called before setStepKillFilter");
  G4DataCollectInternals::s_stepact->setStepKillFilter(sf);
}

void G4DataCollect::finish()
{
  G4RunManager * rm = G4RunManager::GetRunManager();
  if (G4DataCollectInternals::s_evtact)
    {
      G4UserEventAction * existingEvtAct = G4DataCollectInternals::s_evtact->otherAction();
      rm->SetUserAction(existingEvtAct);
      delete G4DataCollectInternals::s_evtact;
    }
  if (G4DataCollectInternals::s_stepact)
    {
      G4UserSteppingAction * existingStepAct = G4DataCollectInternals::s_stepact->otherAction();
      rm->SetUserAction(existingStepAct);
      delete G4DataCollectInternals::s_stepact;//This also closes any open output file
    }
}

void G4DataCollect::setMetaData(const std::string& key,const std::string& value)
{
  assert(G4DataCollectInternals::s_stepact&&"installHooks not called before setMetaData");
  if (key.size()>0&&key[0]=='^') {
    printf("G4DataCollect::setMetaData ERROR: Meta Data keys can not start with '^'\n");
    assert(false);
    exit(1);
  }
  G4DataCollectInternals::s_stepact->setMetaData(key,value);
}

void G4DataCollect::setUserData(const std::string& key,const std::string& value)
{
  assert(G4DataCollectInternals::s_stepact&&"installHooks not called before setUserData");
  static std::string tmp;
  tmp.clear();
  tmp+="^";
  tmp+=key;
  G4DataCollectInternals::s_stepact->setMetaData(tmp.c_str(),value);
}

