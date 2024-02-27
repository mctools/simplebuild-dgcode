#include "EvtFile/DBStringsReader.hh"
#include "Utils/ByteStream.hh"

EvtFile::DBStringsReader::DBStringsReader(subsectid_type subSectionID)
  : m_subSectionID(subSectionID)
{
  m_stringdb.reserve(32);
}

void EvtFile::DBStringsReader::load(const char*& data)
{
  std::uint16_t version;
  ByteStream::read(data,version);
  assert(version==0);//only supported version for now!
  std::uint16_t nstrings_raw;
  ByteStream::read(data,nstrings_raw);
  unsigned nstrings = nstrings_raw;

  if (!nstrings)
    return;

  unsigned oldnstr = m_stringdb.size();
  m_stringdb.resize(oldnstr+nstrings);
  str_type* it = &(m_stringdb[oldnstr]);
  str_type* itE = std::next(it,nstrings);

  while(it!=itE)
    ByteStream::read(data,*(it++));

}
