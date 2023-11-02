#ifndef GriffFormat_DumpFile_hh
#define GriffFormat_DumpFile_hh

//Functions which can be used to dump info about a file to stdout. It is used in
//the command line dumpfile script.

#include "GriffFormat/Format.hh"
#include "EvtFile/DumpFile.hh"

namespace GriffFormat {

  bool dumpFileInfo( const char* filename, bool brief = false, bool show_uncompressed_sizes = false );

  //The following are expected to be used in a piping context, so error messages
  //will go to stderr:
  bool dumpFileEventDBSection( const char* filename, unsigned evtIndex  );
  bool dumpFileEventBriefDataSection( const char* filename, unsigned evtIndex );
  bool dumpFileEventFullDataSection( const char* filename, unsigned evtIndex );

}

#include "GriffFormat/DumpFile.icc"

#endif
