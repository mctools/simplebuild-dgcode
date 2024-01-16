#include "GriffDataRead/Isotope.hh"
#include "GriffDataRead/GriffDataReader.hh"

GriffDataRead::Isotope::Isotope(const char*&data)
  : m_dr(0)
{
  //data will not be available for the entire lifetime of object,
  //so it must be decoded immediately:

  int32_t version; ByteStream::read(data,version);
  assert(version==0);//only supported version

  ByteStream::read(data,m_nameIdx);
  ByteStream::read(data,m_Z);
  ByteStream::read(data,m_N);
  ByteStream::read(data,m_A);
  ByteStream::read(data,m_m);
}

const std::string& GriffDataRead::Isotope::getName() const
{
  assert(m_dr);
  return m_dr->m_dbIsotopeNames.getString(m_nameIdx);
}
