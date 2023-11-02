#ifndef EvtFile_IDBSubSectionWriter_hh
#define EvtFile_IDBSubSectionWriter_hh

#include "EvtFile/FileWriter.hh"
#include "EvtFile/Defs.hh"

namespace EvtFile {

  class IDBSubSectionWriter : public IFWPreFlushCB {
  public:

    IDBSubSectionWriter(FileWriter&);

    //Must be unique within a file format:
    virtual subsectid_type uniqueSubSectionID() const = 0;

    //If there is data to write:
    virtual bool needsWrite() const = 0;

    //Write out any scheduled and previously unwritten data (in the DB section
    //of the FileWriter):
    virtual void write(FileWriter&) = 0;

  private:
    void aboutToFlushEventToDisk(FileWriter&);

  };

}

#endif
