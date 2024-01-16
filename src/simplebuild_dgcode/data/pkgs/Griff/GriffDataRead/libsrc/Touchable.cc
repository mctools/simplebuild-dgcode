#include "GriffDataRead/Touchable.hh"
#include "GriffDataRead/GriffDataReader.hh"

GriffDataRead::Touchable::Touchable(const char*&data)
{
  //data will not be available for the entire lifetime of touchable object,
  //so it must be decoded immediately:
  std::uint8_t depth8bit;
  ByteStream::read(data,depth8bit);
  size_t depth = depth8bit;
  m_info.resize(depth);

  for (unsigned i=0;i<depth;++i) {
    auto& info = m_info[i];
    ByteStream::read(data,info.copyNbr);
    ByteStream::read(data,info.volNameIdx);
    ByteStream::read(data,info.physVolNameIdx);
    ByteStream::read(data,info.materialIdx);
    info.material = 0;
  }
  m_volNameIdx0 = m_info.at(0).volNameIdx;
}

const GriffDataRead::Material * GriffDataRead::Touchable::material(unsigned idepth,GriffDataReader*dr) const {
  assert(idepth<m_info.size());
  const Material * mat = m_info[idepth].material;
  if (mat)
    return mat;
  mat = &(dr->m_dbMaterials.getEntry(m_info[idepth].materialIdx));
  if (!mat->m_dr)
    const_cast<GriffDataRead::Material*>(mat)->m_dr = dr;
  return m_info[idepth].material = mat;
}
