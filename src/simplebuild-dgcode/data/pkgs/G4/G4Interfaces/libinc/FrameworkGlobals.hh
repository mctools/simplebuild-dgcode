#ifndef G4Interfaces_FrameworkGlobals_hh
#define G4Interfaces_FrameworkGlobals_hh

#include "Core/Types.hh"
#include <vector>
#include <sys/types.h>

namespace FrameworkGlobals {

  //Random seed info for current event (might be useful to put in error
  //messages, embed in output files, etc.):
  std::uint64_t currentEvtSeed();

  //Multi-process info (mpID is useful for constructing uniquely named output
  //files, etc.):
  bool isForked();
  bool isParent();
  bool isChild();
  unsigned mpID();
  unsigned nProcs();

  //Global print prefix for printing within the framework:
  const char * printPrefix();

  //////////////////////// Setters ////////////////////////////////
  //Method to be used only by the seeding framework:
  void setCurrentEvtSeed(std::uint64_t&);
  //Methods to be used only by the multi-process framework:
  void setMpID(unsigned);//0 for parent, 1 .. Nproc-1 for childs
  void setNProcs(unsigned);
  //Method to be used only by launcher/multi-process framework:
  void setPrintPrefix(const char*);
}

#endif
