#include "GriffDataRead/Step.hh"
#include "GriffDataRead/DumpObj.hh"

const std::string& GriffDataRead::Step::stepStatusStr() const
{
  static std::string status_strings[8];
  static bool needs_init = true;
  if (needs_init) {
    needs_init = false;
    status_strings[0] = "WorldBoundary";
    status_strings[1] = "GeomBoundary";
    status_strings[2] = "AtRestDoItProc";
    status_strings[3] = "AlongStepDoItProc";
    status_strings[4] = "PostStepDoItProc";
    status_strings[5] = "UserDefinedLimit";
    status_strings[6] = "ExclusivelyForcedProc";
    status_strings[7] = "Undefined";
  }
  unsigned s(stepStatus_raw());
  assert(s<8);
  return status_strings[s];
}
