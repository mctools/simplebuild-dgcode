#include "G4Interfaces/ComponentHandles.hh"
#include <mutex>
#include <vector>

namespace G4Interfaces {
  namespace {
    struct ComponentTable {
      std::mutex mtx;
      std::vector<Utils::ParametersBase*> components;
    };
    ComponentTable& componentTable()
    {
      static ComponentTable t;
      return t;
    }
  }
}

std::int64_t G4Interfaces::registerComponent( Utils::ParametersBase* c )
{
  if ( !c )
    throw std::invalid_argument("registerComponent: null pointer");
  auto& t = componentTable();
  std::lock_guard<std::mutex> guard( t.mtx );
  t.components.push_back( c );
  return static_cast<std::int64_t>( t.components.size() );//handles start at 1
}

Utils::ParametersBase& G4Interfaces::getComponent( std::int64_t handle )
{
  auto& t = componentTable();
  std::lock_guard<std::mutex> guard( t.mtx );
  if ( handle < 1 || handle > static_cast<std::int64_t>( t.components.size() ) )
    throw std::out_of_range("Invalid component handle");
  return *t.components[ static_cast<std::size_t>( handle - 1 ) ];
}
