#ifndef G4DataCollect_DBTouchableEntry_hh
#define G4DataCollect_DBTouchableEntry_hh

#include "EvtFile/IDBEntry.hh"
#include "Core/Types.hh"

//These two before the private public hack, for gcc 6.1.1 on fedora 24Ö
#include "G4String.hh"
#include "G4Types.hh"

//A little private/public hack to access fName directly, since G4LogicalVolume::GetName
//returns by value. Reported as G4 bug in the hope they will change this so we can
//skip the hack: http://bugzilla-geant4.kek.jp/show_bug.cgi?id=1757
//Update: Seems to be fixed in g4 10.2, so can soon get rid of this.
#define private public
#include "G4LogicalVolume.hh"
#undef private

#include "G4TouchableHandle.hh"
#include "G4VPhysicalVolume.hh"

//A DB entry of a "touchable", here defined as the name and copy number of the
//volume, the mother volume and the grandmother volume. It is to be followed by
//a DBStringsWriter subsection with the volume names.

namespace G4DataCollectInternals {

  struct DCMgr;

  class DBTouchableEntry : public EvtFile::IDBEntry {

  public:

    //For efficiency we keep pointers to the char arrays (and assume they have a
    //sufficiently long lifetime - i.e. that the geometry is unchanging), and
    //only convert them to indices + store them in the volume-names string
    //section when they need to be written out. Thus, each object needs to keep
    //a reference to the DBStringsWriter object in charge of the volume names
    //section.

    DBTouchableEntry(DCMgr*, const G4TouchableHandle& th);
    void write(EvtFile::FileWriter&);

#ifdef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
    const char* name() const { return m_vols[0].nameWorkaround.c_str(); }//for debugging
#else
    const char* name() const { return m_vols[0].name->c_str(); }//for debugging
#endif

  protected:

    virtual ~DBTouchableEntry();

    unsigned calculateHash() const;
    bool lessThan(const IDBEntry& o) const;
    bool equals(const IDBEntry& o) const;

  private:
    DBTouchableEntry( const DBTouchableEntry & );
    DBTouchableEntry & operator= ( const DBTouchableEntry & );

    static const unsigned volDepth = 99;//number must fit in 8 bits
    DCMgr * m_mgr;
    G4VPhysicalVolume * m_pv;
    struct VolInfo
    {
      VolInfo()
        :
#ifndef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
        name(0), physName(0),
#endif
        copyNbr(-999), material(0) {}
#ifndef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
      const G4String* name;
      const G4String* physName;
#endif
      G4int copyNbr;
      G4Material* material;
#ifdef GRIFF_EXTRA_TESTS
      std::string nameTest;
      std::string physNameTest;
#endif
#ifdef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
      std::string nameWorkaround;
      std::string physNameWorkaround;
#endif
    };
    VolInfo m_vols[volDepth];
#ifdef GRIFF_EXTRA_TESTS
    static long s_numberActiveInstances;
    unsigned m_actualVolDepthTest;
  public:
    void extraTests() const;
    static long numberActiveInstances() { return s_numberActiveInstances; }
#endif
  };

}

#include "DBTouchableEntry.icc"

#endif
