#include "G4Interfaces/ComponentHandles.hh"
#include <mutex>
#include <vector>

namespace G4Interfaces {
  namespace {
    struct Entry {
      Utils::ParametersBase* component;
      bool owned;
    };
    struct ComponentTable {
      std::mutex mtx;
      std::vector<Entry> entries;
      ~ComponentTable()
      {
        for ( auto it = entries.rbegin(); it != entries.rend(); ++it )
          if ( it->owned )
            delete it->component;
      }
    };
    ComponentTable& componentTable()
    {
      static ComponentTable t;
      return t;
    }
    std::int64_t addEntry( Utils::ParametersBase* c, bool owned )
    {
      if ( !c )
        throw std::invalid_argument("registerComponent: null pointer");
      auto& t = componentTable();
      std::lock_guard<std::mutex> guard( t.mtx );
      t.entries.push_back( Entry{ c, owned } );
      return static_cast<std::int64_t>( t.entries.size() );//handles start at 1
    }
  }
}

std::int64_t G4Interfaces::registerComponent( Utils::ParametersBase* c )
{
  return addEntry( c, true );
}

std::int64_t G4Interfaces::registerUnownedComponent( Utils::ParametersBase* c )
{
  return addEntry( c, false );
}

void G4Interfaces::disownComponent( const Utils::ParametersBase* c )
{
  auto& t = componentTable();
  std::lock_guard<std::mutex> guard( t.mtx );
  for ( auto& e : t.entries )
    if ( e.component == c )
      e.owned = false;
}

Utils::ParametersBase& G4Interfaces::getComponent( std::int64_t handle )
{
  auto& t = componentTable();
  std::lock_guard<std::mutex> guard( t.mtx );
  if ( handle < 1 || handle > static_cast<std::int64_t>( t.entries.size() ) )
    throw std::out_of_range("Invalid component handle");
  return *t.entries[ static_cast<std::size_t>( handle - 1 ) ].component;
}
