#include "GriffB10Common/Utils.hh"

bool GriffB10Common::getPrimaryNeutronAndGeantino(GriffDataReader& dr,
                                                  const GriffDataRead::Track*& trk_neutron,
                                                  const GriffDataRead::Track*& trk_geantino)
{
  if (dr.nPrimaryTracks()!=2)
    return false;
  trk_neutron = dr.primaryTrackBegin();
  trk_geantino = trk_neutron +1;
  if (trk_neutron->pdgCode()!=2112)
    std::swap(trk_neutron,trk_geantino);
  if (trk_neutron->pdgCode()!=2112)
    return false;
  if (trk_geantino->pdgCode()!=999)
    return false;
  return true;
}

const double* GriffB10Common::firstVolIntersectionPos(const GriffDataRead::Track*trk,
                                                      const std::string& volname)
{
  for (auto seg = trk->segmentBegin(); seg!=trk->segmentEnd(); ++seg)
    if (seg->volumeName()==volname)
      return seg->firstStep()->preGlobalArray();
  return 0;
}

const double* GriffB10Common::firstVolIntersectionPos(const GriffDataRead::Track*trk,
                                                      const std::string& volname,
                                                      int volume_copynumber)
{
  for (auto seg = trk->segmentBegin(); seg!=trk->segmentEnd(); ++seg)
    if (volume_copynumber==seg->volumeCopyNumber() && seg->volumeName()==volname )
      return seg->firstStep()->preGlobalArray();
  return 0;
}





