#include "GriffAnaUtils/TrackFilter_PDGCode.hh"

GriffAnaUtils::TrackFilter_PDGCode::TrackFilter_PDGCode()
  : m_unsigned(false)
{
}

GriffAnaUtils::TrackFilter_PDGCode::TrackFilter_PDGCode(int32_t c1)
  : m_unsigned(false)
{
  addCode(c1);
}

GriffAnaUtils::TrackFilter_PDGCode::TrackFilter_PDGCode(int32_t c1,int32_t c2)
  : m_unsigned(false)
{
  addCodes(c1,c2);
}

GriffAnaUtils::TrackFilter_PDGCode::TrackFilter_PDGCode(int32_t c1,int32_t c2,int32_t c3)
  : m_unsigned(false)
{
  addCodes(c1,c2,c3);
}

GriffAnaUtils::TrackFilter_PDGCode::TrackFilter_PDGCode(int32_t c1,int32_t c2,int32_t c3,int32_t c4)
  : m_unsigned(false)
{
  addCodes(c1,c2,c3,c4);
}

GriffAnaUtils::TrackFilter_PDGCode::TrackFilter_PDGCode(int32_t c1,int32_t c2,int32_t c3,int32_t c4,int32_t c5)
  : m_unsigned(false)
{
  addCodes(c1,c2,c3,c4,c5);
}

GriffAnaUtils::TrackFilter_PDGCode * GriffAnaUtils::TrackFilter_PDGCode::addCode(int32_t c)
{
  if (c<0) {
    m_negativeCodes.insert(c);
    m_allAbsCodes.insert(-c);
  } else {
    m_positiveCodes.insert(c);
    m_allAbsCodes.insert(c);
  }
  return this;
}

GriffAnaUtils::TrackFilter_PDGCode * GriffAnaUtils::TrackFilter_PDGCode::addCodes(int32_t c)
{
  return addCode(c);
}

GriffAnaUtils::TrackFilter_PDGCode * GriffAnaUtils::TrackFilter_PDGCode::addCodes(int32_t c1,int32_t c2)
{
  addCode(c1);
  return addCode(c2);
}

GriffAnaUtils::TrackFilter_PDGCode * GriffAnaUtils::TrackFilter_PDGCode::addCodes(int32_t c1,int32_t c2,int32_t c3)
{
  addCodes(c1,c2); return addCode(c3);
}

GriffAnaUtils::TrackFilter_PDGCode * GriffAnaUtils::TrackFilter_PDGCode::addCodes(int32_t c1,int32_t c2,int32_t c3,int32_t c4)
{
  addCodes(c1,c2,c3);
  return addCode(c4);
}

GriffAnaUtils::TrackFilter_PDGCode * GriffAnaUtils::TrackFilter_PDGCode::addCodes(int32_t c1,int32_t c2,int32_t c3,int32_t c4,int32_t c5)
{
  addCodes(c1,c2,c3,c4);
  return addCode(c5);
}
