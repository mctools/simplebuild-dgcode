#ifndef G4DataCollect_DBMetaDataEntry_hh
#define G4DataCollect_DBMetaDataEntry_hh

#include "EvtFile/IDBEntry.hh"
#include "Utils/StringSort.hh"
#include <map>
#include <string>

namespace G4DataCollectInternals {

  struct DCMgr;

  class DBMetaDataEntry : public EvtFile::IDBEntry {

  public:

    //For efficiency we keep pointers to the G4Isotopes (and assume they have a
    //sufficiently long lifetime - i.e. that the geometry is unchanging), and
    //only convert them to indices + store them in the relevant sections when
    //they need to be written out. Thus, each object needs to keep a reference
    //to the DCMgr instance as well.

    //NB: For simplicity we copy the whole map. This is OK since it likely only
    //happens once per job and std::strings are COW.
    DBMetaDataEntry(const std::map<std::string,std::string,Utils::fast_str_cmp>&data,DCMgr*mgr)
      : m_data(data),m_mgr(mgr) {}
    void write(EvtFile::FileWriter&);

  protected:

    //comparisons are purely based on the address of the kept G4Isotopes, thus a
    //user who creates a large number of instances of identical isotopes will be
    //penalised (moral: don't do this!):
    unsigned calculateHash() const;
    bool lessThan(const IDBEntry& o) const { return m_data < static_cast<const DBMetaDataEntry*>(&o)->m_data; }
    bool equals(const IDBEntry& o) const { return m_data == static_cast<const DBMetaDataEntry*>(&o)->m_data; }

  private:
    DBMetaDataEntry( const DBMetaDataEntry & );
    DBMetaDataEntry & operator= ( const DBMetaDataEntry & );
    const std::map<std::string,std::string,Utils::fast_str_cmp> m_data;
    DCMgr * m_mgr;
  };

}

#endif
