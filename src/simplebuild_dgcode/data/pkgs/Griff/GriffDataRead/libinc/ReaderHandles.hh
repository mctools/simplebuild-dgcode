#ifndef GriffDataRead_ReaderHandles_hh
#define GriffDataRead_ReaderHandles_hh

#include <cstdint>

class GriffDataReader;

namespace GriffDataRead {

  //Table of GriffDataReader objects identified by integer handles. This allows
  //Python modules (which can not pass objects between each other) to refer to
  //the same reader. The readers are not owned by the table, and must be
  //unregistered before they are deleted.

  std::int64_t registerReader( GriffDataReader* );
  void unregisterReader( std::int64_t handle );
  //Throws std::invalid_argument for unknown handles:
  GriffDataReader& getReader( std::int64_t handle );

}

#endif
