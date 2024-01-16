#include "GriffDataRead/Element.hh"
#include "GriffDataRead/GriffDataReader.hh"

GriffDataRead::Element::Element(const char*&data)
  : m_dr(0)
{
  //data will not be available for the entire lifetime of object,
  //so it must be decoded immediately:

  int32_t version; ByteStream::read(data,version);
  assert(version==0);//only supported version

  ByteStream::read(data,m_nameIdx);
  ByteStream::read(data,m_symbolIdx);
  ByteStream::read(data,m_Z);
  ByteStream::read(data,m_N);
  ByteStream::read(data,m_A);
  std::uint32_t na;
  ByteStream::read(data,na);
  assert(na==0||na==1);
  m_naturalabundances = bool(na);

  std::uint32_t nisotopes;
  ByteStream::read(data,nisotopes);
  m_isotopes.resize(nisotopes);
  auto isoIt = m_isotopes.begin();
  for (unsigned i = 0; i<nisotopes; ++i,++isoIt) {
    ByteStream::read(data,isoIt->relativeabundance);
    ByteStream::read(data,isoIt->index);
    isoIt->object = 0;
  }
}

const std::string& GriffDataRead::Element::getName() const
{
  assert(m_dr);
  return m_dr->m_dbElementNames.getString(m_nameIdx);
}

const std::string& GriffDataRead::Element::getSymbol() const
{
  assert(m_dr);
  return m_dr->m_dbElementNames.getString(m_symbolIdx);
}

const GriffDataRead::Isotope* GriffDataRead::Element::getIsotope(unsigned i_isotope) const
{
  assert(i_isotope<m_isotopes.size());
  IsotopeInfo& ii = m_isotopes[i_isotope];
  if (!ii.object) {
    assert(m_dr);
    ii.object = &(m_dr->m_dbIsotopes.getEntry(ii.index));
    if (!(ii.object->m_dr))
      const_cast<Isotope*>(ii.object)->m_dr = m_dr;
  }
  assert(ii.object&&m_isotopes[i_isotope].object);
  return ii.object;
}
