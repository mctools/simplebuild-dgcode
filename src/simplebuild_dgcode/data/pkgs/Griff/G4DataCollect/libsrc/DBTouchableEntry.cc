#include "DBTouchableEntry.hh"
#include "DBMaterialEntry.hh"
#include "DCMgr.hh"
#include "EvtFile/FileWriter.hh"
#include "EvtFile/Defs.hh"

#ifdef GRIFF_EXTRA_TESTS
#include <stdexcept>
long G4DataCollectInternals::DBTouchableEntry::s_numberActiveInstances = 0;
void G4DataCollectInternals::DBTouchableEntry::extraTests() const
{
  unsigned actualDepth(volDepth);
  for (;actualDepth>0;--actualDepth) {
    assert(actualDepth-1<volDepth);
#ifdef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
    if (!m_vols[actualDepth-1].nameWorkaround.empty())
      break;
#else
    if (m_vols[actualDepth-1].name!=0)
      break;
#endif
  }
  if (actualDepth!=m_actualVolDepthTest) {
    throw std::runtime_error("GRIFF_EXTRA_TESTS depth failure!");
  }
  for (unsigned i = 0; i<actualDepth; ++i) {
    const VolInfo & vi = m_vols[i];
#ifdef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
    if (vi.nameTest != vi.nameWorkaround||vi.physNameTest != vi.physNameWorkaround )
      throw std::runtime_error("GRIFF_EXTRA_TESTS name length failure despite workaround!");
#else
    if (vi.nameTest != *(vi.name) ||vi.physNameTest != *(vi.physName) )
      throw std::runtime_error("GRIFF_EXTRA_TESTS name length failure!");
#endif
  }
}
#endif

G4DataCollectInternals::DBTouchableEntry::~DBTouchableEntry()
{
#ifdef GRIFF_EXTRA_TESTS
  extraTests();
  --s_numberActiveInstances;
#endif
}

G4DataCollectInternals::DBTouchableEntry::DBTouchableEntry(DCMgr*mgr, const G4TouchableHandle& th)
  : m_mgr(mgr),m_pv(0)
{
#ifdef GRIFF_EXTRA_TESTS
  ++s_numberActiveInstances;
  m_actualVolDepthTest = 0;
#endif
  int thdepth = th->GetHistoryDepth() + 1;
  assert(thdepth>=1);
  unsigned vd(volDepth);
  unsigned maxDepth(std::min(vd,(unsigned)thdepth));
  for (unsigned ivol=0;ivol<maxDepth;++ivol) {
    auto physvol = th->GetVolume(ivol);
    if (ivol==0)
      m_pv=physvol;
    assert(physvol);
    if (!physvol)
      break;
    VolInfo & vi = m_vols[ivol];
    auto logvol = physvol->GetLogicalVolume();
#ifdef GRIFF_EXTRA_TESTS
    ++m_actualVolDepthTest;
    vi.nameTest = logvol->GetName();
    vi.physNameTest = physvol->GetName();
#endif
#ifdef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
    vi.nameWorkaround = logvol->GetName();
    vi.physNameWorkaround = physvol->GetName();
#else
    vi.name = &(logvol->fName);
    vi.physName = &(physvol->GetName());
#endif
    vi.copyNbr = th->GetCopyNumber(ivol);
    vi.material = logvol->GetMaterial();
  }
#ifdef GRIFF_EXTRA_TESTS
  extraTests();
#endif
}

void G4DataCollectInternals::DBTouchableEntry::write(EvtFile::FileWriter&fw)
{
#ifdef GRIFF_EXTRA_TESTS
  extraTests();
#endif
#ifndef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
  static std::string tmp;
#endif

  unsigned actualDepth(volDepth);
  for (;actualDepth>0;--actualDepth) {
    assert(actualDepth-1<volDepth);
#ifdef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
    if (!m_vols[actualDepth-1].nameWorkaround.empty())
      break;
#else
    if (m_vols[actualDepth-1].name!=0)
      break;
#endif
  }
  assert(actualDepth<=volDepth);
  fw.writeDataDBSection((std::uint8_t)actualDepth);
  for (unsigned i = 0; i<actualDepth; ++i) {
#ifdef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
    assert(m_vols[i].nameWorkaround.size()<2048);
    assert(m_vols[i].physNameWorkaround.size()<2048);
    EvtFile::index_type logIdx = m_mgr->dbVolNames.getIndex(m_vols[i].nameWorkaround);
    EvtFile::index_type physIdx = m_mgr->dbVolNames.getIndex(m_vols[i].physNameWorkaround);
#else
    assert(m_vols[i].name->size()<2048);
    assert(m_vols[i].physName->size()<2048);
    EvtFile::index_type logIdx = m_mgr->dbVolNames.getIndex(*(m_vols[i].name));
    EvtFile::index_type physIdx = m_mgr->dbVolNames.getIndex(*(m_vols[i].physName));
#endif
    fw.writeDataDBSection((int32_t)m_vols[i].copyNbr);
    fw.writeDataDBSection(logIdx);
    fw.writeDataDBSection(physIdx);

    DBMaterialEntry * matentry = new DBMaterialEntry(m_vols[i].material,m_mgr);
    matentry->ref();
    fw.writeDataDBSection(m_mgr->dbMaterials.getIndex(matentry));
    matentry->unref();
  }
}

bool G4DataCollectInternals::DBTouchableEntry::lessThan(const IDBEntry& oe) const
{
  assert(dynamic_cast<const DBTouchableEntry *>(&oe));
  const DBTouchableEntry * o =static_cast<const DBTouchableEntry *>(&oe);

#ifdef GRIFF_EXTRA_TESTS
  extraTests();
  o->extraTests();
#endif

  if (m_pv!=o->m_pv)
    return m_pv<o->m_pv;

  for (unsigned i=0;i<volDepth;++i) {
    const VolInfo & vi = m_vols[i];
    const VolInfo & o_vi = o->m_vols[i];
    if (vi.copyNbr != o_vi.copyNbr)
      return vi.copyNbr < o_vi.copyNbr;
    if (vi.material != o_vi.material)
      return vi.material < o_vi.material;
#ifdef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
    if (vi.nameWorkaround!=o_vi.nameWorkaround)
      return (vi.nameWorkaround<o_vi.nameWorkaround);
    if (vi.physNameWorkaround!=o_vi.physNameWorkaround)
      return (vi.physNameWorkaround<o_vi.physNameWorkaround);
#else
    if ( vi.name != o_vi.name && *(vi.name) != *(o_vi.name) )
      return ( *(vi.name) < *(o_vi.name) );
    if ( vi.physName != o_vi.physName && *(vi.physName) != *(o_vi.physName) )
      return ( *(vi.physName) < *(o_vi.physName) );
#endif
  }
#ifdef GRIFF_EXTRA_TESTS
  assert(this->equals(oe));
#endif
  return false;//equals
}

bool G4DataCollectInternals::DBTouchableEntry::equals(const IDBEntry& oe) const
{
  assert(dynamic_cast<const DBTouchableEntry *>(&oe));
  const DBTouchableEntry * o =static_cast<const DBTouchableEntry *>(&oe);

#ifdef GRIFF_EXTRA_TESTS
  extraTests();
  o->extraTests();
#endif

  if (m_pv!=o->m_pv)
    return false;
  for (unsigned i=0;i<volDepth;++i) {
    const VolInfo & vi = m_vols[i];
    const VolInfo & o_vi = o->m_vols[i];
    if (vi.copyNbr != o_vi.copyNbr)
      return false;
    if (vi.material != o_vi.material)
      return false;
#ifdef GRIFF_APPLY_WORKAROUND_FOR_NAMEBUG
    if (vi.nameWorkaround!=o_vi.nameWorkaround)
      return false;
    if (vi.physNameWorkaround!=o_vi.physNameWorkaround)
      return false;
#else
    if ( vi.name != o_vi.name && *(vi.name) != *(o_vi.name) )
      return false;
    if ( vi.physName != o_vi.physName && *(vi.physName) != *(o_vi.physName) )
      return false;
#endif
  }
  return true;
}
