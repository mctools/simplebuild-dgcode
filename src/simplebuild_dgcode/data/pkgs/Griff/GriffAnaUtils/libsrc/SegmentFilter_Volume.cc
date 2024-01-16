#include "GriffAnaUtils/SegmentFilter_Volume.hh"

GriffAnaUtils::SegmentFilter_Volume::SegmentFilter_Volume(const char* volumeName,
                                                            const char* motherVolumeName,
                                                            const char* grandMotherVolumeName)
  : m_depth(0), m_logical(true)

{
  if (volumeName) {
    m_depth = 1;
    m_volname[0] = volumeName;
    if (motherVolumeName) {
      m_depth = 2;
      m_volname[1] = motherVolumeName;
      if (grandMotherVolumeName) {
        m_depth = 3;
        m_volname[2] = grandMotherVolumeName;
      }
    } else {
      assert(!grandMotherVolumeName);
    }
  } else {
    assert(!motherVolumeName);
    assert(!grandMotherVolumeName);
  }
}
