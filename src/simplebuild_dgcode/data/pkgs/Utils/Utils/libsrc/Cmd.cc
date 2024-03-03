#include "Utils/Cmd.hh"
#include <array>
#include <stdexcept>
#include <cstdio>

namespace {
  struct UtilsProcWrapper {
    //RAII wrapper
    FILE * procptr = nullptr;
    UtilsProcWrapper( const char * cmd )
      : procptr(popen(cmd,"r"))
    {
    }
    int close_and_get_rc()
    {
      if ( !procptr )
        throw std::runtime_error("launch_cmd: unexpected error");
      auto pp = procptr;
      procptr = nullptr;
      return pclose( pp );
    }
    ~UtilsProcWrapper()
    {
      if ( procptr )
        pclose( procptr );
    }
  };
}

std::pair<int, std::string> Utils::launch_cmd( const char* cmd,
                                               std::size_t reserve_size )
{
  std::array<char, 256> buf;
  std::pair<int, std::string> res;
  if (reserve_size)
    res.second.reserve( reserve_size );
  UtilsProcWrapper theproc( cmd );
  if ( theproc.procptr != nullptr ) {
    while ( fgets( buf.data(), buf.size(), theproc.procptr ) != nullptr )
      res.second += buf.data();
  }
  res.first = theproc.close_and_get_rc();
  return res;
}
