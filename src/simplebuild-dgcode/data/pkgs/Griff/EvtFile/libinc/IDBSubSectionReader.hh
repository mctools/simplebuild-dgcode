#ifndef EvtFile_IDBSubSectionReader_hh
#define EvtFile_IDBSubSectionReader_hh

#include "EvtFile/Defs.hh"

namespace EvtFile {

  class FileReader;

  class IDBSubSectionReader {
  public:
    IDBSubSectionReader(){}
    virtual ~IDBSubSectionReader();

    //Must be unique within a file format:
    virtual subsectid_type uniqueSubSectionID() const = 0;

    //Read data. When done, the data pointer must have advanced to the next
    //unread byte (it is the responsibility of the reader/writer pair to know
    //how much to read of the stream):
    virtual void load(const char*&) = 0;
    virtual void clearInfo() = 0;//must clear all loaded info upon request (normally when opening a new file)

  private:
    IDBSubSectionReader( const IDBSubSectionReader & );
    IDBSubSectionReader & operator= ( const IDBSubSectionReader & );
  };

}

#endif
