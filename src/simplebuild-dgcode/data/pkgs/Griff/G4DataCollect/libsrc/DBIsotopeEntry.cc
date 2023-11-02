#include "DBIsotopeEntry.hh"
#include "DCMgr.hh"

void G4DataCollectInternals::DBIsotopeEntry::write(EvtFile::FileWriter&fw)
{
  fw.writeDataDBSection((int32_t)0);//version
  fw.writeDataDBSection(m_mgr->dbIsotopeNames.getIndex(m_isotope->GetName()));//name
  fw.writeDataDBSection(int32_t(m_isotope->GetZ()));//Z
  fw.writeDataDBSection(int32_t(m_isotope->GetN()));//N
  fw.writeDataDBSection(double(m_isotope->GetA()));//A (atomic mass)
  fw.writeDataDBSection(int32_t(m_isotope->Getm()));//isomer level
}
