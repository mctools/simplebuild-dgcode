#ifndef Utils_Cmd_hh
#define Utils_Cmd_hh

#include <utility>
#include <string>

namespace Utils {
  //Launch command and get both exit code and stdout of command. The
  //reserve_size parameter can be used to reserve capacity in the output string,
  //but is only needed for a very small performance gain:
  std::pair<int, std::string> launch_cmd( const char* cmd,
                                          std::size_t reserve_size = 0 );
}

#endif
