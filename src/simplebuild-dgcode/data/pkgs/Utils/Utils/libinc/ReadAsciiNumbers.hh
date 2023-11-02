#ifndef Utils_ReadAsciiNumbers_hh
#define Utils_ReadAsciiNumbers_hh

// A very simple function which reads numbers from a text file into a vector,
// dealing correctly with eof and whitespace.

#include <fstream>
#include <vector>
#include <string>

namespace Utils {

  template<class TValue>
  inline bool read_numbers(const std::string& filename, std::vector<TValue>& v)
  {
    std::ifstream f(filename);
    while( !(f>>std::ws).eof() ) {
      TValue p;
      f >> p;
      if (f.fail())
        return false;
      v.emplace_back(p);
    }
    return true;
  }

}

#endif
