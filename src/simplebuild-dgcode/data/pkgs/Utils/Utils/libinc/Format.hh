#ifndef Utils_Format_hh
#define Utils_Format_hh

#include <string>
#include <cstdio>
#include <cstdarg>

namespace Utils {
  void string_format(std::string& output, const std::string fmt, ...);
}

#endif
