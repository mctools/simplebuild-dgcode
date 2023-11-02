#ifndef Utils_StrSwitch_hh
#define Utils_StrSwitch_hh

// Hack to allow switching on strings, like this:
//
//  switchstr(mystr) {
//    casestr("opt1") : do_opt1(); break;
//    casestr("opt2") : do_opt2(); break;
//  }
//

#include <cstdint>
#include <string>

namespace StrSwitch {

  inline constexpr std::uint64_t hash_compile_time(char const* str, std::uint64_t last_value = 0xCBF29CE484222325ull)
  {
    return *str ? hash_compile_time(str+1, (*str ^ last_value) * 0x100000001B3ull) : last_value;
  }

  //runtime versions:

  inline std::uint64_t hash(const std::string& str)
  {
    return hash_compile_time(str.c_str());
  }

  inline std::uint64_t hash(const char * str)
  {
    return hash_compile_time(str);
  }

}
#define switchstr(str) switch(StrSwitch::hash(str))
#define casestr(strlit) case StrSwitch::hash_compile_time(strlit)

#endif
