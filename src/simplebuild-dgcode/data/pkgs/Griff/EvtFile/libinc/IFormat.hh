#ifndef EvtFile_Format_hh
#define EvtFile_Format_hh

#include "Core/Types.hh"

//Derive from the Format class to define a data format using the container
//provided by the EvtFile package.

namespace EvtFile {

  class IFormat
  {
  public:
    //The magic word which all datafiles will start with.
    virtual std::uint32_t magicWord() const = 0;

    //Extension of data files (will be appended automatically by FileWriter):
    virtual const char* fileExtension() const = 0;

    //Names of brief and full data sections as they will be referred to in
    //output of dumpfiles, etc.:
    virtual const char* eventBriefDataName() const = 0;
    virtual const char* eventFullDataName() const = 0;

    virtual bool compressFullData() const { return false; }

  };
}

#endif
