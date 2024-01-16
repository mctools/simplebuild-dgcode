#include "GriffDataRead/Material.hh"
#include "GriffDataRead/GriffDataReader.hh"

GriffDataRead::Material::Material(const char*&data)
  : m_dr(0)
{
  //data will not be available for the entire lifetime of object,
  //so it must be decoded immediately:

  int32_t version; ByteStream::read(data,version);
  assert(version==0);//only supported version

  ByteStream::read(data,m_nameIdx);
  ByteStream::read(data,m_density);
  ByteStream::read(data,m_temperature);
  ByteStream::read(data,m_pressure);
  ByteStream::read(data,m_radlen);
  ByteStream::read(data,m_nuclearinterlength);
  ByteStream::read(data,m_ion_meanexcitationenergy);
  ByteStream::read(data,m_state);
  assert(m_state>=0&&m_state<=3);
  std::uint32_t nelem;
  ByteStream::read(data,nelem);
  m_elements.resize(nelem);
  auto elemIt = m_elements.begin();
  for (unsigned i = 0; i<nelem; ++i,++elemIt) {
    ByteStream::read(data,elemIt->fraction);
    ByteStream::read(data,elemIt->index);
    elemIt->object = 0;
  }
}

const std::string& GriffDataRead::Material::getName() const
{
  assert(m_dr);
  return m_dr->m_dbMaterialNames.getString(m_nameIdx);
}

const GriffDataRead::Element* GriffDataRead::Material::getElement(unsigned ielem) const
{
  assert(ielem<m_elements.size());
  ElementInfo& ei = m_elements[ielem];
  if (!ei.object) {
    assert(m_dr);
    ei.object = &(m_dr->m_dbElements.getEntry(ei.index));
    if (!(ei.object->m_dr))
      const_cast<Element*>(ei.object)->m_dr = m_dr;
  }
  assert(ei.object&&m_elements[ielem].object);
  return ei.object;
}

const std::string& GriffDataRead::Material::stateStr() const
{
  static std::string state_strings[4];
  static bool needs_init = true;
  if (needs_init) {
    needs_init = false;
    state_strings[0] = "Undefined";
    state_strings[1] = "Solid";
    state_strings[2] = "Liquid";
    state_strings[3] = "Gas";
  }
  assert(m_state>=0&&m_state<=3);
  return state_strings[m_state];
}
