#ifndef EvtFile_DBStringsReader_hh
#define EvtFile_DBStringsReader_hh

//Handles string-storage in the database section (you can have more than one
//instance for strings from separate unrelated sources).

#include "EvtFile/IDBSubSectionReader.hh"
#include <cassert>
#include "Utils/StringSort.hh"
#include <string>
#include <vector>

namespace EvtFile {

  class DBStringsReader final : public IDBSubSectionReader {
  public:
    DBStringsReader(subsectid_type subSectionID);
    ~DBStringsReader(){}

    DBStringsReader( const DBStringsReader& ) = delete;
    DBStringsReader& operator=( const DBStringsReader& ) = delete;
    DBStringsReader( DBStringsReader&& ) = delete;
    DBStringsReader& operator=( DBStringsReader&& ) = delete;

    //Access the string DB:
    const str_type& getString( index_type i ) const
    {
      assert(i<m_stringdb.size());
      return m_stringdb[i];
    }

    //For the data loading framework:
    virtual subsectid_type uniqueSubSectionID() const { return m_subSectionID; }
    virtual void load(const char*&);
    virtual void clearInfo() { m_stringdb.clear(); }

  private:
    //Indices are loaded consecutively and starting from zero => We can use a vector:
    std::vector<str_type> m_stringdb;
    subsectid_type m_subSectionID;
  };

}

#endif

