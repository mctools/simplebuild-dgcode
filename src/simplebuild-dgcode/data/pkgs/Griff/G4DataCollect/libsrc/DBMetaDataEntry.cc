#include "DBMetaDataEntry.hh"
#include "DCMgr.hh"

unsigned G4DataCollectInternals::DBMetaDataEntry::calculateHash() const {
  unsigned tmphash = m_data.size()*1048576;
  unsigned i=1;
  auto itE=m_data.end();
  for (auto it=m_data.begin();it!=itE;++it)
    tmphash += (i++)*(it->first.size()+1024*it->second.size());
  return tmphash;
}

void G4DataCollectInternals::DBMetaDataEntry::write(EvtFile::FileWriter&fw)
{
  fw.writeDataDBSection((int32_t)0);//version
  fw.writeDataDBSection((std::uint32_t)m_data.size());//size
  auto itE=m_data.end();
  for (auto it=m_data.begin();it!=itE;++it) {
    fw.writeDataDBSection(m_mgr->dbMetaDataStrings.getIndex(it->first));//key
    fw.writeDataDBSection(m_mgr->dbMetaDataStrings.getIndex(it->second));//value
  }
}
