#include "G4Interfaces/FrameworkGlobals.hh"
#include <string>
#include "G4Materials/NamedMaterialProvider.hh"

namespace FrameworkGlobals {

  static std::uint64_t s_currentEvtSeed = 0;
  std::uint64_t currentEvtSeed() { return s_currentEvtSeed; }
  void setCurrentEvtSeed(std::uint64_t& s) { s_currentEvtSeed = s; }

  static int s_mpID = INT_MAX;
  void setMpID(unsigned mpid) { s_mpID = mpid; }
  bool isForked() { return s_mpID!=INT_MAX; }
  bool isChild()  { return s_mpID>0&&s_mpID<INT_MAX; }
  bool isParent() { return !isChild(); }
  unsigned mpID() { return s_mpID; }

  static unsigned s_nprocs = 1;
  void setNProcs(unsigned n) { s_nprocs = n; }
  unsigned nProcs() { return s_nprocs; }

  //Global print prefix:
  static std::string s_printPrefix = "";
  const char * printPrefix() { return s_printPrefix.c_str(); }
  void setPrintPrefix(const char* p) { s_printPrefix = p; NamedMaterialProvider::setPrintPrefix(p); }
}
