#include "sbldpy/sbldpy.hh"
#include "GriffFormat/DumpFile.hh"
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace {
  unsigned toEvtIndex( std::int64_t evtIndex )
  {
    if ( evtIndex < 0 || evtIndex > std::numeric_limits<unsigned>::max() )
      throw std::out_of_range("evtIndex out of range");
    return static_cast<unsigned>(evtIndex);
  }
}

SBLDPY_MODULE( _dumpfile )
{
  m.def( "dumpFileInfo",
         []( const char* filename, bool brief, bool show_uncompressed_sizes )
         { return GriffFormat::dumpFileInfo( filename, brief, show_uncompressed_sizes ); },
         "dump content of G4 event file",
         { "filename", "brief", "show_uncompressed_sizes" } );

  m.def( "dumpFileEventDBSection",
         []( const char* filename, std::int64_t evtIndex )
         { return GriffFormat::dumpFileEventDBSection( filename, toEvtIndex( evtIndex ) ); },
         "binary dump to stdout of the content of the database section of evt at evtIndex position in Griff file",
         { "filename", "evtIndex" } );

  m.def( "dumpFileEventBriefDataSection",
         []( const char* filename, std::int64_t evtIndex )
         { return GriffFormat::dumpFileEventBriefDataSection( filename, toEvtIndex( evtIndex ) ); },
         "binary dump to stdout of the content of the brief data section of evt at evtIndex position in Griff file",
         { "filename", "evtIndex" } );

  m.def( "dumpFileEventFullDataSection",
         []( const char* filename, std::int64_t evtIndex )
         { return GriffFormat::dumpFileEventFullDataSection( filename, toEvtIndex( evtIndex ) ); },
         "binary dump to stdout of the content of the full data section of evt at evtIndex position in Griff file",
         { "filename", "evtIndex" } );
}
