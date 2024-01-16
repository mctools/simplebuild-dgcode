#include "GriffDataRead/MetaData.hh"
#include "GriffDataRead/GriffDataReader.hh"

GriffDataRead::MetaData::MetaData(const char*&data)
{
  //data will not be available for the entire lifetime of object,
  //so it must be decoded immediately:

  int32_t version; ByteStream::read(data,version);
  assert(version==0);//only supported version

  std::uint32_t n; ByteStream::read(data,n);
  m_data.resize(n);
  for (unsigned i=0;i<n;++i) {
    ByteStream::read(data,m_data[i].first);
    ByteStream::read(data,m_data[i].second);
  }
}

void GriffDataRead::MetaData::initCache(GriffDataReader* dr) const
{
  assert(dr&&m_cache.empty());
  auto itE = m_data.end();
  for (auto it = m_data.begin(); it!=itE; ++it)
    m_cache[dr->m_dbMetaDataStrings.getString(it->first)]= dr->m_dbMetaDataStrings.getString(it->second);
  assert(m_cache.size()==m_data.size());
}
