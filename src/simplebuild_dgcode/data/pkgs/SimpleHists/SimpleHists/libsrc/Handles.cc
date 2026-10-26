#include "SimpleHists/Handles.hh"
#include "SimpleHists/HistCollection.hh"
#include <map>
#include <mutex>
#include <stdexcept>

namespace SimpleHists {
  namespace {
    template<class T>
    struct Table {
      std::mutex mtx;
      std::map<std::int64_t,std::shared_ptr<T>> objects;
      std::int64_t lastHandle = 0;
      std::int64_t add( std::shared_ptr<T> o )
      {
        if ( !o )
          throw std::invalid_argument( "Can not register null object" );
        std::lock_guard<std::mutex> guard( mtx );
        objects[++lastHandle] = std::move( o );
        return lastHandle;
      }
      std::shared_ptr<T> get( std::int64_t handle )
      {
        std::lock_guard<std::mutex> guard( mtx );
        auto it = objects.find( handle );
        if ( it == objects.end() )
          throw std::invalid_argument( "Unknown SimpleHists handle" );
        return it->second;
      }
      void remove( std::int64_t handle )
      {
        std::shared_ptr<T> o;//(deleted after the lock is released)
        std::lock_guard<std::mutex> guard( mtx );
        auto it = objects.find( handle );
        if ( it != objects.end() ) {
          o = std::move( it->second );
          objects.erase( it );
        }
      }
    };
    Table<HistBase>& histTable() { static Table<HistBase> t; return t; }
    Table<HistCollection>& collectionTable() { static Table<HistCollection> t; return t; }
  }
}

std::int64_t SimpleHists::registerHist( std::shared_ptr<HistBase> h ) { return histTable().add( std::move( h ) ); }
std::shared_ptr<SimpleHists::HistBase> SimpleHists::getHist( std::int64_t handle ) { return histTable().get( handle ); }
void SimpleHists::unregisterHist( std::int64_t handle ) { histTable().remove( handle ); }

std::int64_t SimpleHists::registerCollection( std::shared_ptr<HistCollection> c ) { return collectionTable().add( std::move( c ) ); }
std::shared_ptr<SimpleHists::HistCollection> SimpleHists::getCollection( std::int64_t handle ) { return collectionTable().get( handle ); }
void SimpleHists::unregisterCollection( std::int64_t handle ) { collectionTable().remove( handle ); }
