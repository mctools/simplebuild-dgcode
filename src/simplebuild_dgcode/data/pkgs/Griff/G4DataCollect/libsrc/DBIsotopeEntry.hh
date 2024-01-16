#ifndef G4DataCollect_DBIsotopeEntry_hh
#define G4DataCollect_DBIsotopeEntry_hh

#include "EvtFile/IDBEntry.hh"
class G4Isotope;

namespace G4DataCollectInternals {

  struct DCMgr;

  class DBIsotopeEntry : public EvtFile::IDBEntry {

  public:

    //For efficiency we keep pointers to the G4Isotopes (and assume they have a
    //sufficiently long lifetime - i.e. that the geometry is unchanging), and
    //only convert them to indices + store them in the relevant sections when
    //they need to be written out. Thus, each object needs to keep a reference
    //to the DCMgr instance as well.

    DBIsotopeEntry(const G4Isotope*isotope,DCMgr*mgr) : m_isotope(isotope), m_mgr(mgr) {}

    void write(EvtFile::FileWriter&);

  protected:

    //comparisons are purely based on the address of the kept G4Isotopes, thus a
    //user who creates a large number of instances of identical isotopes will be
    //penalised (moral: don't do this!):
    unsigned calculateHash() const { return reinterpret_cast<intptr_t>(m_isotope); }
    bool lessThan(const IDBEntry& o) const { return m_isotope < static_cast<const DBIsotopeEntry*>(&o)->m_isotope; }
    bool equals(const IDBEntry& o) const { return m_isotope == static_cast<const DBIsotopeEntry*>(&o)->m_isotope; }

  private:
    DBIsotopeEntry( const DBIsotopeEntry & );
    DBIsotopeEntry & operator= ( const DBIsotopeEntry & );
    const G4Isotope * m_isotope;
    DCMgr * m_mgr;
  };

}

#endif
