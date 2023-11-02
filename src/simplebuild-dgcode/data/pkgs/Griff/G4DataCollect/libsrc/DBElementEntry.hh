#ifndef G4DataCollect_DBElementEntry_hh
#define G4DataCollect_DBElementEntry_hh

#include "EvtFile/IDBEntry.hh"
class G4Element;

namespace G4DataCollectInternals {

  struct DCMgr;

  class DBElementEntry : public EvtFile::IDBEntry {

  public:

    //For efficiency we keep pointers to the G4Elements (and assume they have a
    //sufficiently long lifetime - i.e. that the geometry is unchanging), and
    //only convert them to indices + store them in the relevant sections when
    //they need to be written out. Thus, each object needs to keep a reference
    //to the DCMgr instance as well.

    DBElementEntry(const G4Element*elem,DCMgr*mgr) : m_elem(elem), m_mgr(mgr) {}

    void write(EvtFile::FileWriter&);

  protected:

    //comparisons are purely based on the address of the kept G4Elements, thus
    //a user who creates a large number of instances of identical elements will
    //be penalised (moral: don't do this!):
    unsigned calculateHash() const { return reinterpret_cast<intptr_t>(m_elem); }
    bool lessThan(const IDBEntry& o) const { return m_elem < static_cast<const DBElementEntry*>(&o)->m_elem; }
    bool equals(const IDBEntry& o) const { return m_elem == static_cast<const DBElementEntry*>(&o)->m_elem; }

  private:
    DBElementEntry( const DBElementEntry & );
    DBElementEntry & operator= ( const DBElementEntry & );
    const G4Element * m_elem;
    DCMgr * m_mgr;
  };

}

#endif
