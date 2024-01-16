#ifndef EvtFile_EvtFileDefs_hh
#define EvtFile_EvtFileDefs_hh

//The file format version we are currently writing (the container version, not
//the version of the contained data):
#define EVTFILE_VERSION ((int32_t)2)

//Sizes in EVTFILE__VERSION 0,1,2:
#define EVTFILE_FILE_HEADER_BYTES (2*sizeof(int32_t))
#define EVTFILE_EVENT_HEADER_BYTES (6*sizeof(std::uint32_t))

#endif
