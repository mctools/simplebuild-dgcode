#ifndef EvtFile_IDBEntry_hh
#define EvtFile_IDBEntry_hh

//Base class to be used when building databases of more complex data-types with
//DBEntryWriter. It ensure lookup efficiency by hash utilisation and takes care
//of ownership issues by refcounting. NB: When reading from databases no
//specific baseclass is necessary, see DBEntryReader.

#include "Core/Types.hh"
#include <cassert>

namespace EvtFile {

  class FileWriter;
  class DBEntryWriter;

  class IDBEntry {
  public:

    IDBEntry();

    IDBEntry( const IDBEntry& ) = delete;
    IDBEntry& operator=( const IDBEntry& ) = delete;
    IDBEntry( IDBEntry&& ) = delete;
    IDBEntry& operator=( IDBEntry&& ) = delete;

    //When writing:
    virtual void write(EvtFile::FileWriter&) = 0;//stream data to db section

    bool operator<(const IDBEntry& o) const;
    bool operator==(const IDBEntry& o) const;
    bool operator!=(const IDBEntry& o) const;
    void ref();
    void unref();
    unsigned refCount() const { return m_refCount; }
    unsigned hash() const { return m_hash; }
  protected:
    //Will be called once by the framework (after object is fully constructed):
    virtual unsigned calculateHash() const = 0;
    //implement full (i.e. not based on the hash) comparison:
    virtual bool lessThan(const IDBEntry& o) const = 0;
    virtual bool equals(const IDBEntry& o) const = 0;
    virtual ~IDBEntry() {}
  private:
    // IDBEntry( const IDBEntry & );
    // IDBEntry & operator= ( const IDBEntry & );
    unsigned m_hash;
    unsigned m_refCount;
    friend class DBEntryWriter;
    void setHash();
  };

  struct dbentry_cmp {
    //Speed up maps/sets with string keys by sorting based on hash before
    //contents are considered.
    template<class Tdbtypeptr>
    bool operator()(const Tdbtypeptr& s1,const Tdbtypeptr& s2) const
    {
      return s1->hash()==s2->hash() ? *s1<*s2 : s1->hash()<s2->hash();
    }
  };

}

#include "EvtFile/IDBEntry.icc"

#endif
