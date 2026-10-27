#include "G4Interfaces/CppObjectHandles.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4UserSteppingAction.hh"
#include "G4UserEventAction.hh"
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace G4Interfaces {
  namespace {

    enum class Kind { SteppingAction, EventAction, PreGenHook, PostGenHook, HookFct };

    const char * kindName( Kind k )
    {
      switch ( k ) {
      case Kind::SteppingAction: return "G4UserSteppingAction";
      case Kind::EventAction: return "G4UserEventAction";
      case Kind::PreGenHook: return "PreGenCallBack";
      case Kind::PostGenHook: return "PostGenCallBack";
      case Kind::HookFct: return "HookFct";
      };
      return "unknown";
    }

    struct Entry {
      Kind kind;
      void * rawptr = nullptr;//for the user actions
      void (*rawdeleter)( void* ) = nullptr;
      std::shared_ptr<void> sharedptr;//for the hooks
      bool claimed = false;
    };

    struct ObjectTable {
      std::mutex mtx;
      std::vector<Entry> entries;
      ~ObjectTable()
      {
        //Delete unclaimed user actions (hooks are deleted by their shared_ptr):
        for ( auto it = entries.rbegin(); it != entries.rend(); ++it )
          if ( it->rawptr )
            it->rawdeleter( it->rawptr );
      }
    };

    ObjectTable& objectTable()
    {
      static ObjectTable t;
      return t;
    }

    std::int64_t addEntry( Entry e )
    {
      if ( !e.rawptr && !e.sharedptr )
        throw std::invalid_argument("registerObject: null pointer");
      auto& t = objectTable();
      std::lock_guard<std::mutex> guard( t.mtx );
      t.entries.push_back( std::move( e ) );
      return static_cast<std::int64_t>( t.entries.size() );//handles start at 1
    }

    //Must be called with the mutex locked:
    Entry& getEntry( ObjectTable& t, std::int64_t handle )
    {
      if ( handle < 1 || handle > static_cast<std::int64_t>( t.entries.size() ) )
        throw std::out_of_range("Invalid object handle");
      return t.entries[ static_cast<std::size_t>( handle - 1 ) ];
    }

    Entry claimEntry( std::int64_t handle, Kind kind )
    {
      auto& t = objectTable();
      std::lock_guard<std::mutex> guard( t.mtx );
      auto& e = getEntry( t, handle );
      if ( e.kind != kind )
        throw std::invalid_argument( std::string("Object is a ") + kindName( e.kind )
                                     + " and not a " + kindName( kind ) );
      if ( e.claimed )
        throw std::invalid_argument( std::string("The ") + kindName( e.kind )
                                     + " object was already used (it can only be used once)" );
      e.claimed = true;
      Entry res = e;
      e.rawptr = nullptr;
      e.sharedptr.reset();
      return res;
    }

    template<class T>
    Entry sharedEntry( Kind k, std::shared_ptr<T> p )
    {
      Entry e;
      e.kind = k;
      e.sharedptr = std::move( p );
      return e;
    }

    template<class T>
    Entry rawEntry( Kind k, T * p )
    {
      Entry e;
      e.kind = k;
      e.rawptr = p;
      e.rawdeleter = []( void * ptr ) { delete static_cast<T*>( ptr ); };
      return e;
    }

  }
}

std::int64_t G4Interfaces::registerObject( G4UserSteppingAction* p )
{
  return addEntry( rawEntry( Kind::SteppingAction, p ) );
}

std::int64_t G4Interfaces::registerObject( G4UserEventAction* p )
{
  return addEntry( rawEntry( Kind::EventAction, p ) );
}

std::int64_t G4Interfaces::registerObject( std::shared_ptr<PreGenCallBack> p )
{
  return addEntry( sharedEntry( Kind::PreGenHook, std::move( p ) ) );
}

std::int64_t G4Interfaces::registerObject( std::shared_ptr<PostGenCallBack> p )
{
  return addEntry( sharedEntry( Kind::PostGenHook, std::move( p ) ) );
}

std::int64_t G4Interfaces::registerObject( std::shared_ptr<HookFct> p )
{
  return addEntry( sharedEntry( Kind::HookFct, std::move( p ) ) );
}

G4UserSteppingAction* G4Interfaces::claimUserSteppingAction( std::int64_t handle )
{
  return static_cast<G4UserSteppingAction*>( claimEntry( handle, Kind::SteppingAction ).rawptr );
}

G4UserEventAction* G4Interfaces::claimUserEventAction( std::int64_t handle )
{
  return static_cast<G4UserEventAction*>( claimEntry( handle, Kind::EventAction ).rawptr );
}

std::shared_ptr<G4Interfaces::PreGenCallBack> G4Interfaces::claimPreGenHook( std::int64_t handle )
{
  return std::static_pointer_cast<PreGenCallBack>( claimEntry( handle, Kind::PreGenHook ).sharedptr );
}

std::shared_ptr<G4Interfaces::PostGenCallBack> G4Interfaces::claimPostGenHook( std::int64_t handle )
{
  return std::static_pointer_cast<PostGenCallBack>( claimEntry( handle, Kind::PostGenHook ).sharedptr );
}

std::shared_ptr<G4Interfaces::HookFct> G4Interfaces::claimHookFct( std::int64_t handle )
{
  return std::static_pointer_cast<HookFct>( claimEntry( handle, Kind::HookFct ).sharedptr );
}

const char * G4Interfaces::objectKind( std::int64_t handle )
{
  auto& t = objectTable();
  std::lock_guard<std::mutex> guard( t.mtx );
  return kindName( getEntry( t, handle ).kind );
}

bool G4Interfaces::objectIsClaimed( std::int64_t handle )
{
  auto& t = objectTable();
  std::lock_guard<std::mutex> guard( t.mtx );
  return getEntry( t, handle ).claimed;
}
