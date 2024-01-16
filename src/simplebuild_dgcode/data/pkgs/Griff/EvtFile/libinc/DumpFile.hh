#ifndef EvtFile_DumpFile_hh
#define EvtFile_DumpFile_hh

//Functions which can be used to dump info about a file to stdout when it is
//provided info about the expected file format.

#include "EvtFile/IFormat.hh"

namespace EvtFile {

  //TODO: bool isFormat(const Format*, const char* filename );

  bool dumpFileInfo(const IFormat*, const char* filename, bool brief = false, bool show_uncompressed_sizes=false );

  //The following are expected to be used in a piping context, so error messages
  //will go to stderr:
  bool dumpFileEventDBSection(const IFormat*, const char* filename, unsigned evtIndex  );
  bool dumpFileEventBriefDataSection(const IFormat*, const char* filename, unsigned evtIndex );
  bool dumpFileEventFullDataSection(const IFormat*, const char* filename, unsigned evtIndex );

}

#endif
