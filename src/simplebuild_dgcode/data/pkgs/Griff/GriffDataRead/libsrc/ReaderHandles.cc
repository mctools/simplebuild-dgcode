#include "GriffDataRead/ReaderHandles.hh"
#include <map>
#include <mutex>
#include <stdexcept>

namespace GriffDataRead {
  namespace {
    struct ReaderTable {
      std::mutex mtx;
      std::map<std::int64_t,GriffDataReader*> readers;
      std::int64_t lastHandle = 0;
    };
    ReaderTable& readerTable()
    {
      static ReaderTable table;
      return table;
    }
  }
}

std::int64_t GriffDataRead::registerReader( GriffDataReader* dr )
{
  auto& t = readerTable();
  std::lock_guard<std::mutex> guard( t.mtx );
  t.readers[++t.lastHandle] = dr;
  return t.lastHandle;
}

void GriffDataRead::unregisterReader( std::int64_t handle )
{
  auto& t = readerTable();
  std::lock_guard<std::mutex> guard( t.mtx );
  t.readers.erase( handle );
}

GriffDataReader& GriffDataRead::getReader( std::int64_t handle )
{
  auto& t = readerTable();
  std::lock_guard<std::mutex> guard( t.mtx );
  auto it = t.readers.find( handle );
  if ( it == t.readers.end() )
    throw std::invalid_argument( "Unknown GriffDataReader handle (the reader might have been deleted)" );
  return *it->second;
}
