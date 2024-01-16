#ifndef Utils_Glob_hh
#define Utils_Glob_hh

//Not very efficient (but convenient) wrapper around glob
#include <glob.h>
#include <vector>
#include <string>

namespace Utils {
  //appends any glob(pattern) results to "out" vector
  void glob(const std::string& pattern,std::vector<std::string>& out);
}

#endif
