#ifndef EvtFile_DBStringsWriter_hh
#define EvtFile_DBStringsWriter_hh

//Handles string-storage in the database section (you can have more than one
//instance for strings from separate unrelated sources).

#include "EvtFile/IDBSubSectionWriter.hh"
#include <cassert>
#include "Utils/StringSort.hh"
#include <string>
#include <vector>
#include <map>

namespace EvtFile {

  class DBStringsWriter final : public IDBSubSectionWriter {
  public:
    DBStringsWriter(subsectid_type subSectionID, FileWriter& fw)
      : IDBSubSectionWriter(fw),
        m_nextIndex(0),
        m_subSectionID(subSectionID)
    {}
    ~DBStringsWriter(){}

    DBStringsWriter( const DBStringsWriter& ) = delete;
    DBStringsWriter& operator=( const DBStringsWriter& ) = delete;
    DBStringsWriter( DBStringsWriter&& ) = delete;
    DBStringsWriter& operator=( DBStringsWriter&& ) = delete;

    virtual subsectid_type uniqueSubSectionID() const { return m_subSectionID; }

    //Get unique identifier of string (this will schedule the string for next
    //write operation if not previously done):
    index_type getIndex( const str_type& s );

    //Write out any scheduled and previously unwritten data (in the DB section
    //of the FileWriter):
    virtual void write(FileWriter& fw);

    virtual bool needsWrite() const
    { return !m_addedSinceLastWrite.empty(); }

  private:

    typedef std::map<str_type,index_type,Utils::fast_str_cmp> map_type;
    static const unsigned NMAPS = 32;//reduce map lookup time by keeping several maps and looking in m_map[str.size()%NMAPS]
    index_type m_nextIndex;
    map_type m_map[NMAPS];
    std::vector<map_type::const_iterator> m_addedSinceLastWrite;
    subsectid_type m_subSectionID;
  };

  #include "DBStringsWriter.icc"
}

#endif

