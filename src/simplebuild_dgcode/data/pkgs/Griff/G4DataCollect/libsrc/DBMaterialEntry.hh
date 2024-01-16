#ifndef G4DataCollect_DBMaterialEntry_hh
#define G4DataCollect_DBMaterialEntry_hh

#include "EvtFile/IDBEntry.hh"
class G4Material;

namespace G4DataCollectInternals {

  struct DCMgr;

  class DBMaterialEntry : public EvtFile::IDBEntry {

  public:

    //For efficiency we keep pointers to the G4Materials (and assume they have a
    //sufficiently long lifetime - i.e. that the geometry is unchanging), and
    //only convert them to indices + store them in the relevant sections when
    //they need to be written out. Thus, each object needs to keep a reference
    //to the DCMgr instance as well.

    DBMaterialEntry(const G4Material*mat,DCMgr*mgr) : m_mat(mat), m_mgr(mgr) {}

    void write(EvtFile::FileWriter&);

  protected:

    //comparisons are purely based on the address of the kept G4Materials, thus
    //a user who creates a large number of instances of identical materials will
    //be penalised (moral: don't do this!):
    unsigned calculateHash() const { return reinterpret_cast<intptr_t>(m_mat); }
    bool lessThan(const IDBEntry& o) const { return m_mat < static_cast<const DBMaterialEntry*>(&o)->m_mat; }
    bool equals(const IDBEntry& o) const { return m_mat == static_cast<const DBMaterialEntry*>(&o)->m_mat; }

  private:
    DBMaterialEntry( const DBMaterialEntry & );
    DBMaterialEntry & operator= ( const DBMaterialEntry & );
    const G4Material * m_mat;
    DCMgr * m_mgr;
  };

}

#endif
