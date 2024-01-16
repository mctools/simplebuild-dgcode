#include "MCPL/mcpl.h"
#include "Core/FindData.hh"

int main(int argc,char**argv) {
  //Avoid unused warnings (can't simply omit the names in C):
  (void)argc;
  (void)argv;

  //NB: reading of reffile_crash.mcpl.gz is not supported.
  mcpl_dump(core_finddata("MCPLTestsFMT2","reffile_2.mcpl.gz"),0,0,0);
  mcpl_dump(core_finddata("MCPLTestsFMT2","reffile_5.mcpl.gz"),0,0,0);
  mcpl_dump(core_finddata("MCPLTestsFMT2","reffile_empty.mcpl.gz"),0,0,0);
}
