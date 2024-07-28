#ifndef EvtFile_DBEntryReader_hh
#define EvtFile_DBEntryReader_hh

//Sister class of DBEntryWriter for reading back in the custom entries. Note
//that the template class does not need any specific baseclass, the only
//requirement is that it has a constructor of the form:
//
//              Tentry(const char*& data)
//
//Which initialises itself by reading off bytes from the data stream and leaving
//data pointing at the next unread byte.

#include "EvtFile/IDBSubSectionReader.hh"
#include "EvtFile/Defs.hh"
#include "Utils/ByteStream.hh"
#include <vector>

namespace EvtFile {

  template<class Tentry>
  class DBEntryReader final : public IDBSubSectionReader {
  public:

    DBEntryReader(subsectid_type subSectionID)
      : m_subSectionID(subSectionID) {}
    ~DBEntryReader(){}

    DBEntryReader( const DBEntryReader& ) = delete;
    DBEntryReader& operator=( const DBEntryReader& ) = delete;
    DBEntryReader( DBEntryReader&& ) = delete;
    DBEntryReader& operator=( DBEntryReader&& ) = delete;

    //Access the DB:
    const Tentry& getEntry( index_type i )
    {
      assert(i<m_db.size());
      return *(m_db[i]);
    }

    //For the data loading framework:
    virtual subsectid_type uniqueSubSectionID() const { return m_subSectionID; }
    virtual void load(const char*&);
    virtual void clearInfo();

  private:

    std::vector<Tentry*> m_db;
    subsectid_type m_subSectionID;

  };

}

#include "EvtFile/DBEntryReader.icc"

#endif
