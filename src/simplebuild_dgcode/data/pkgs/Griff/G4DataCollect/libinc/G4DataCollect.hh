#ifndef G4DataCollect_hh
#define G4DataCollect_hh

#include <string>

namespace G4Interfaces { class StepFilterBase; }
class G4UserEventAction;
class G4UserSteppingAction;

//////////////////////////////////////////////

class G4DataCollect {
public:
  //Will install hooks into the G4RunManager in order to collect data with a
  //G4UserSteppingAction and write them at the end of each event with
  //G4UserEventAction. It will allow actions already installed with the
  //G4RunManager to still function, so be sure to install any other
  //G4UserSteppingAction and G4UserEventAction *before* calling
  //installHooks. Alternatively, add them later with the
  //installUserSteppingAction and installUserEventAction *after* calling
  //installHooks.
  //
  //outputFile will automatically get the extension ".griff" appended.
  //
  //Use the mode parameter to adjust amount of step information written
  //(tradeoff between file-size and available information):
  //
  //  FULL: Write out all steps
  //  REDUCED: Coalesce steps following each other in the same volume into one.
  //  MINIMAL: No step info, only tracks and segments summaries.
  //
  //See documentation for further details about the file format (TODO)

  static void installHooks(const char* outputFile, const char* mode = "FULL");

  static void installUserSteppingAction(G4UserSteppingAction*);
  static void installUserEventAction(G4UserEventAction*);

  //removes hooks again and closes output file:
  static void finish();

  //The next two methods can be used to store job settings, etc., inside the
  //files (think of it as a custom map<string,string> which you can fill with
  //whatever you feel like). The values stay active for all following events
  //until overridden. Note that technically it doesn't really matter whether you
  //store data as "MetaData" or "UserData", but the former should be reserved
  //for use by the simulation framework and the latter for individual
  //simulations to store specific data. G4DataCollect will prefill a few G4
  //related values in the MetaData section (version, data lib locations, etc.):
  static void setMetaData(const std::string& key,const std::string& value);
  static void setUserData(const std::string& key,const std::string& value);

  //Optionally provide a step-filter [G4DataCollect takes ownership]
  static void setStepFilter(G4Interfaces::StepFilterBase*);

  //... and/or a kill filter [G4DataCollect takes ownership]:
  static void setStepKillFilter(G4Interfaces::StepFilterBase*);

private:
  struct Imp;
};

#endif
