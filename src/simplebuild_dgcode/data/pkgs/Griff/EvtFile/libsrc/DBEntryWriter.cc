#include "EvtFile/DBEntryWriter.hh"

EvtFile::DBEntryWriter::~DBEntryWriter()
{
  for (unsigned i=0;i<NMAPS;++i) {
    auto it = m_map[i].begin();
    auto itE = m_map[i].end();
    for (;it!=itE;++it)
      it->first->unref();
  }
}

void EvtFile::DBEntryWriter::write(EvtFile::FileWriter&fw )
{
  assert(fw.ok());

  fw.writeDataDBSection((std::uint16_t)0);//version
  assert(m_addedSinceLastWrite.size()<UINT32_MAX);
  fw.writeDataDBSection((std::uint32_t)m_addedSinceLastWrite.size());//number of new objects

  auto it = m_addedSinceLastWrite.begin();
  auto itE = m_addedSinceLastWrite.end();
  for (;it!=itE;++it) {
    auto t = (*it)->first;
    assert(t);
    t->write(fw);
  }
  m_addedSinceLastWrite.clear();
}


//TODO: proper raw dumpfile support, including test using it!
