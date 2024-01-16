#include "EvtFile/DBStringsWriter.hh"
#include "EvtFile/FileWriter.hh"

void EvtFile::DBStringsWriter::write(FileWriter&fw)
{
  assert(fw.ok());

  fw.writeDataDBSection((std::uint16_t)0);//version
  assert(m_addedSinceLastWrite.size()<UINT16_MAX);
  fw.writeDataDBSection((std::uint16_t)m_addedSinceLastWrite.size());//number of new strings

  auto it = m_addedSinceLastWrite.begin();
  auto itE = m_addedSinceLastWrite.end();
  for (;it!=itE;++it) {
    const str_type& s = (*it)->first;
    //64kb should be enough for any sensible string we are going to write!
    std::uint16_t n = s.size()<UINT16_MAX ? s.size() : UINT16_MAX;
    assert(n<UINT16_MAX);
    fw.writeDataDBSection(n);//string length
    if (n)
      fw.writeDataDBSection(&(s[0]),n);//We don't write the null char.
  }
  m_addedSinceLastWrite.clear();
}
