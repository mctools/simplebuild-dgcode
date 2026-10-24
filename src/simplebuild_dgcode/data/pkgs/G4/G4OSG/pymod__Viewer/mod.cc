#include "sbldpy/sbldpy.hh"
#include "G4OSG/Viewer.hh"
#include <cstdint>
#include <set>
#include <stdexcept>

namespace G4OSG {
  int run_viewer( const char* griff_file, bool show_geo, const std::set<std::uint64_t>& events )
  {
    G4OSG::Viewer * viewer = new G4OSG::Viewer(griff_file,show_geo,events);
    int ec = viewer->run();
    if (ec)
      return ec;
    delete viewer;
    return 0;
  }

  std::set<std::uint64_t> decode_events( sbldpy::NumpyView<const std::int64_t> events )
  {
    std::set<std::uint64_t> res;
    for ( std::size_t i = 0; i < events.size(); ++i ) {
      if ( events[i] < 0 )
        throw std::invalid_argument("Negative event number");
      res.insert( static_cast<std::uint64_t>( events[i] ) );
    }
    return res;
  }
}

SBLDPY_MODULE( _Viewer )
{
  m.def( "run", []() { return G4OSG::run_viewer( nullptr, true, {} ); },
         "Launch viewer of the geometry" );
  m.def( "run_griff",
         []( const char* griff_file, sbldpy::NumpyView<const std::int64_t> events )
         { return G4OSG::run_viewer( griff_file, true, G4OSG::decode_events( events ) ); },
         "Launch viewer of the geometry and the events in a Griff file (all events if no events are given)",
         { "griff_file", "events" } );
  m.def( "run_griff_nogeom",
         []( const char* griff_file, sbldpy::NumpyView<const std::int64_t> events )
         { return G4OSG::run_viewer( griff_file, false, G4OSG::decode_events( events ) ); },
         "Launch viewer of the events in a Griff file (all events if no events are given), without the geometry",
         { "griff_file", "events" } );
}
