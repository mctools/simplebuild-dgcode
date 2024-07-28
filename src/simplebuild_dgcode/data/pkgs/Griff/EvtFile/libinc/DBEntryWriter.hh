#ifndef EvtFile_DBEntryWriter_hh
#define EvtFile_DBEntryWriter_hh

//Helper class which makes it easy to build up database sections where entries
//can be arbitrarily complex data structures and needs to be assigned a simple
//unique index (integer) assigned so they can be referenced in other sections.

#include "EvtFile/IDBSubSectionWriter.hh"
#include "EvtFile/IDBEntry.hh"
#include <vector>
#include <map>

namespace EvtFile {

  class IDBEntry;

  class DBEntryWriter final : public IDBSubSectionWriter {
  public:
    typedef std::uint32_t index_type;

    DBEntryWriter(std::uint16_t subSectionID, FileWriter& fw)
      : IDBSubSectionWriter(fw),
        m_nextIndex(0),
        m_subSectionID(subSectionID)
    {}

    DBEntryWriter( const DBEntryWriter& ) = delete;
    DBEntryWriter& operator=( const DBEntryWriter& ) = delete;
    DBEntryWriter( DBEntryWriter&& ) = delete;
    DBEntryWriter& operator=( DBEntryWriter&& ) = delete;

    virtual ~DBEntryWriter();

    virtual std::uint16_t uniqueSubSectionID() const { return m_subSectionID; }

    //Get unique identifier of object (this will schedule it for next write
    //operation if not previously done):
    index_type getIndex( IDBEntry* t );

    //Write out any scheduled and previously unwritten data (in the DB section
    //of the FileWriter):
    virtual void write(FileWriter& fw);

    virtual bool needsWrite() const
    { return !m_addedSinceLastWrite.empty(); }

  private:
    typedef std::map<IDBEntry*,index_type,dbentry_cmp> map_type;
    static const unsigned NMAPS = 32;//reduce map lookup time by keeping several maps and looking in m_map[o.hash()%NMAPS]
    index_type m_nextIndex;
    map_type m_map[NMAPS];
    std::vector<map_type::const_iterator> m_addedSinceLastWrite;
    std::uint16_t m_subSectionID;
  };

}

#include "EvtFile/DBEntryWriter.icc"

#endif
