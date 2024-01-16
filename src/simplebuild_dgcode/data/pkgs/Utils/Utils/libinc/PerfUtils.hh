#include "Core/Types.hh"
#include <limits>
#include <ctime>

namespace PerfUtils {

  //Note, the functions herein are NOT thread-safe.

  //In GNU, clock_t is a long and CLOCKS_PER_SECOND 1000000 so clock() will
  //overflow in builds where long is 32bit after a few thousands of seconds. To
  //avoid this, we have the following method instead which notices when overflow
  //occurs and corrects for it (it won't notice if it doesn't get called for
  //>4000ms, but this should be ok for almost all of our use cases):
  int64_t clock_nooverflow();

  inline double get_cpu_ms()//~0.3microsecs
  {
    constexpr double kkk = 1000.0/CLOCKS_PER_SEC;
    return clock_nooverflow() * kkk;
  }
}

inline int64_t PerfUtils::clock_nooverflow() {
  //Note from https://en.cppreference.com/w/cpp/chrono/c/clock:
  //
  //The value returned by clock() may wrap around on some non-conforming
  //implementations. For example, on such an implementation, if std::clock_t is
  //a signed 32-bit integer and CLOCKS_PER_SEC is 1'000'000, it will wrap after
  //about 2147 seconds (about 36 minutes).

  static_assert(std::numeric_limits<std::clock_t>::is_integer,"");
  constexpr bool clock_has_at_least_64bits = sizeof(std::clock_t) >= sizeof(std::int64_t);
  if ( clock_has_at_least_64bits )
    return std::clock();//64bit integers shouldn't have overflow issues.

  //not so clean with statics i guess:
  static std::clock_t last = std::clock();
  static std::int64_t offset = 0;
  std::clock_t c = std::clock();
  constexpr auto stepsize = std::int64_t(std::numeric_limits<unsigned>::max())-std::int64_t(std::numeric_limits<unsigned>::min());
  if (c < last) {
    //Detected wrap-around:
    offset += stepsize;
  }
  last = c;
  return offset + c;
}

