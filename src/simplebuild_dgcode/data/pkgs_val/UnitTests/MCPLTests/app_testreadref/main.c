#include "MCPL/mcpl.h"
#include "Core/FindData.hh"
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char**argv) {
  //Avoid unused warnings (can't simply omit the names in C):
  (void)argc;
  (void)argv;

  char filename[65];
  for (unsigned count = 1;count<=16;++count) {
    sprintf(filename, "reffile_%i.mcpl",count);
    mcpl_dump(core_finddata("MCPLTests",filename),0,0,0);
  }
  mcpl_dump(core_finddata("MCPLTests","reffile_crash.mcpl"),0,0,0);
  mcpl_dump(core_finddata("MCPLTests","reffile_empty.mcpl"),0,0,0);

  char cmd[1024];
  sprintf(cmd, "/bin/cp %s reffile_7_copy.mcpl",core_finddata("MCPLTests","reffile_7.mcpl"));
  int ec  = system(cmd);
  if (ec)
    return 1;
  mcpl_merge_inplace("reffile_7_copy.mcpl",core_finddata("MCPLTests","reffile_7.mcpl"));
  mcpl_dump("reffile_7_copy.mcpl",0,0,0);
  return ec;
}
