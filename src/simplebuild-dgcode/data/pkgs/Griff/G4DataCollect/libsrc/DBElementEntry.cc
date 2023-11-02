#include "DBElementEntry.hh"
#include "DBIsotopeEntry.hh"
#include "DCMgr.hh"
#include "G4Version.hh"

void G4DataCollectInternals::DBElementEntry::write(EvtFile::FileWriter&fw)
{
  fw.writeDataDBSection((int32_t)0);//version
  fw.writeDataDBSection(m_mgr->dbElementNames.getIndex(m_elem->GetName()));//name
  fw.writeDataDBSection(m_mgr->dbElementNames.getIndex(m_elem->GetSymbol()));//symbol
  fw.writeDataDBSection(double(m_elem->GetZ()));//Z
  fw.writeDataDBSection(double(m_elem->GetN()));//N
  fw.writeDataDBSection(double(m_elem->GetA()));//A
  //natural abundances flag: (nb. the method is just wrapping a bool field, so should have been const!)
#if G4VERSION_NUMBER >= 1000
  fw.writeDataDBSection(std::uint32_t(const_cast<G4Element*>(m_elem)->GetNaturalAbundanceFlag()?1:0));
#else
  fw.writeDataDBSection(std::uint32_t(const_cast<G4Element*>(m_elem)->GetNaturalAbandancesFlag()?1:0));
#endif
  //ignoring atomic shell info
  std::uint32_t niso = m_elem->GetNumberOfIsotopes();
  fw.writeDataDBSection(niso);//number of isotopes
  for (unsigned i=0;i<niso;++i) {
    const G4Isotope * iso = m_elem->GetIsotope(i);
    double relativeabundance = m_elem->GetRelativeAbundanceVector()[i];
    DBIsotopeEntry * isoEntry = new DBIsotopeEntry(iso,m_mgr);
    isoEntry->ref();
    EvtFile::index_type isoIdx = m_mgr->dbIsotopes.getIndex(isoEntry);
    isoEntry->unref();
    fw.writeDataDBSection(relativeabundance);//relativeabundance i
    fw.writeDataDBSection(isoIdx);//isotope index i
  }
}
