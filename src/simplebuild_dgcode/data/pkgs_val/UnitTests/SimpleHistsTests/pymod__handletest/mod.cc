#include "sbldpy/sbldpy.hh"
#include "SimpleHists/Handles.hh"
#include "SimpleHists/HistCollection.hh"
#include <memory>
#include <stdexcept>

//Module for testing the sharing of histograms and collections between modules
//with handles (see scripts/testhandles).

namespace {
  namespace sh = SimpleHists;

  //Fill a histogram created in Python (the very same object is filled):
  void fillHist( std::int64_t handle, double value )
  {
    auto h = dynamic_cast<sh::Hist1D*>( sh::getHist( handle ).get() );
    if ( !h )
      throw std::invalid_argument( "not a Hist1D" );
    h->fill( value );
  }

  //Histogram created in C++, handed over to Python:
  std::int64_t createHist()
  {
    auto h = std::make_shared<sh::Hist1D>( "made in C++", 10, 0.0, 10.0 );
    h->fill( 2.5 );
    h->fill( 3.5, 2.0 );
    return sh::registerHist( h );
  }

  //Book a histogram in a collection created in Python:
  void bookInCollection( std::int64_t handle, const std::string& key )
  {
    auto c = sh::getCollection( handle );
    c->book1D( "booked in C++", 5, 0.0, 1.0, key )->fill( 0.3 );
  }

  //Collection created in C++ (keeping a histogram it shares with C++ code
  //below), handed over to Python:
  std::shared_ptr<sh::Hist1D> g_kept;
  std::int64_t createCollection()
  {
    auto c = std::make_shared<sh::HistCollection>();
    c->bookCounts( "counts" );
    g_kept = std::make_shared<sh::Hist1D>( 4, 0.0, 4.0 );
    c->add( g_kept, "shared" );
    return sh::registerCollection( c );
  }
  void fillKept( double value ) { g_kept->fill( value ); }
  void releaseKept() { g_kept.reset(); }
}

SBLDPY_MODULE( _handletest )
{
  m.def( "fill_hist", &fillHist, { "handle", "value" } );
  m.def( "create_hist", &createHist );
  m.def( "book_in_collection", &bookInCollection, { "handle", "key" } );
  m.def( "create_collection", &createCollection );
  m.def( "fill_kept", &fillKept, { "value" } );
  m.def( "release_kept", &releaseKept );
}
