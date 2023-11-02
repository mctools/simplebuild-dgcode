//Test that the following mini-main code for the mcpltool also compiles with C++
//(which is less liberal regarding const-ness when passing in argv):

#include "MCPL/mcpl.h"

int main ( int argc, char** argv )
{
  return mcpl_tool(argc,argv);
}
