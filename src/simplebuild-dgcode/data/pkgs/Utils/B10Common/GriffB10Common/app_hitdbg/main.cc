#include "GriffAnaUtils/All.hh"
#include "Units/Units.hh"
#include "Utils/ArrayMath.hh"
#include "SimpleHistsUtils/AutoBinHistCollection.hh"
#include "GriffB10Common/DetHitApproxFlex.hh"
#include "GriffB10Common/Utils.hh"
#include "GriffB10Common/TrueTOFEstimator.hh"

//Simple analysis which should work on any "CountingGas"+"Converter" simulation which
//was performed with one primary particle, a neutron, and an associated
//co-directional geantino created with the G4GeantinoInserter module.

//TODO: Use weights in fills?

//#define DEBUG_TRUETOF

int main(int argc,char**argv) {

  GriffDataReader dr(argc,argv);

  const std::string volname_conv("Converter");

  SimpleHists::AutoBinHistCollection hc;
  hc.setAutoFitDefault(0.98);//ignore outliers for automatic bin ranges

  auto h_conv_vs_true = hc.book1D("Displacement upon reaching converter [mm]",
                                  100, "conv_vs_true");
  auto h_angular_res_hitfullhighres = hc.book1D("Angular dist of detected hits (FULL, high res) from DetHitapproximation [degree]",
                                                100, "angular_res_hitfullhighres");
  auto h_angular_res_hitreduced = hc.book1D("Angular dist of detected hits (REDUCED) from DetHitapproximation [degree]",
                                                100, "angular_res_hitreduced");
  auto h_hit_vs_conv_std = hc.book1D("Displacement of hit [std hit class] versus conversion point [mm]",
                                     100, "hit_vs_conv_std");
  auto h_hit_vs_conv_full = hc.book1D("Displacement of hit [FULL] versus conversion point [mm]",
                                      100, "hit_vs_conv_full");
  auto h_hit_vs_conv_reduced = hc.book1D("Displacement of hit [REDUCED] versus conversion point [mm]",
                                         100, "hit_vs_conv_reduced");
  auto h_hit_vs_conv_fullhighres = hc.book1D("Displacement of hit [FULL+highres] versus conversion point [mm]",
                                             100, "hit_vs_conv_fullhighres");
  auto h_hit_vs_conv_reducedhighres = hc.book1D("Displacement of hit [REDUCED+highres] versus conversion point [mm]",
                                                100,"hit_vs_conv_reducedhighres");
  auto h_hit_redvsfullhighres = hc.book1D("Displacement of hit locations for REDUCED vs. FULL+highress [mm]",
                                             100, "hit_redvsfullhighres");

  auto h_time_res_hitfullhighres = hc.book1D("Time resolution of detected hits (FULL, high res) [us]",
                                             100, "time_res_hitfullhighres");
  auto h_time_res_hitreduced = hc.book1D("Time resolution of detected hits (REDUCED) [us]",
                                         100, "time_res_hitreduced");

#ifdef DEBUG_TRUETOF
  auto h_time_at_conv_vs_expected = hc.book1D("Time when neutron reaches converter minus expected time in case of no interactions [us]",
                                              100, "time_at_conv_vs_expected");
#endif

  auto h_classify = hc.bookCounts("evt_classify");
  auto c_use = h_classify->addCounter("event_used");
  auto c_skip = h_classify->addCounter("badgenevt");
  auto c_skip_minimal = h_classify->addCounter("badmode");
  auto c_skip_aim = h_classify->addCounter("badaim");
  auto c_skip_neutronmissconv = h_classify->addCounter("neutronmissconv");
  c_use.setComment("Event used for all plots");
  c_skip_neutronmissconv.setComment("Neutron miss converter");
  c_skip.setComment("Event could not be used as it did not have 2 primary particles: 1 neutron & 1 geantino");
  c_skip_minimal.setComment("Event could not be used as it was recorded in MINIMAL mode");
  c_skip_aim.setComment("Event could not be used as geantino did not aim at a converter");

  DetHitApproximation hit_std(&dr);
  DetHitApproxFlex hit_full(&dr);
  DetHitApproxFlex hit_fullhighres(&dr);
  hit_fullhighres.setNPointsPerPath(10);
  DetHitApproxFlex hit_red(&dr);
  hit_red.setPretendReduced();
  DetHitApproxFlex hit_redhighres(&dr);
  hit_redhighres.setPretendReduced();
  hit_redhighres.setNPointsPerPath(10);

  //Loop over events and extract info via Griff interface:
  while (dr.loopEvents()) {
    const GriffDataRead::Track* trk_neutron;
    const GriffDataRead::Track* trk_geantino;
    if (!GriffB10Common::getPrimaryNeutronAndGeantino(dr,trk_neutron,trk_geantino)) {
      ++c_skip;
      continue;
    }
    if (dr.eventStorageMode()==GriffFormat::Format::MODE_MINIMAL) {
      ++c_skip_minimal;
      continue;
    }
    //decode where geantino reaches converter, if somewhere:
    const double* true_conv_pos = GriffB10Common::firstVolIntersectionPos(trk_geantino,volname_conv);
    if (!true_conv_pos) {
      ++c_skip_aim;
      continue;
    }

    ++c_use;

    double true_tof;
    bool has_true_tof = GriffB10Common::estimateTrueTOF(dr,true_tof,volname_conv);

#ifdef DEBUG_TRUETOF
    for (auto seg = trk_neutron->segmentBegin(); seg!=trk_neutron->segmentEnd(); ++seg) {
      if (seg->volumeName()==volname_conv) {
        h_time_at_conv_vs_expected->fill((seg->firstStep()->preTime() - true_tof)/Units::ms);
        break;
      }
    }
#endif


    //Using a few hit helpers, check detected direction and time of all hits
    //(also those not actually arising from conversions):
    auto p0_true = trk_geantino->firstStep()->preGlobalArray();
    auto mom0_true = trk_geantino->firstStep()->preMomentumArray();
    double dir_hit[3];
    if (hit_fullhighres.eventHasHit()) {
      Utils::subtract(hit_fullhighres.eventHitPosition(),p0_true,dir_hit);
      h_angular_res_hitfullhighres->fill(Utils::theta(mom0_true,dir_hit)/Units::deg);
      if (has_true_tof)
        h_time_res_hitfullhighres->fill((hit_fullhighres.eventHitTime()-true_tof)/Units::ms);
    }
    if (hit_red.eventHasHit()) {
      Utils::subtract(hit_red.eventHitPosition(),p0_true,dir_hit);
      h_angular_res_hitreduced->fill(Utils::theta(mom0_true,dir_hit)/Units::deg);
      if (has_true_tof)
        h_time_res_hitreduced->fill((hit_red.eventHitTime()-true_tof)/Units::ms);
    }

    //decode where neutron reaches converter, if somewhere:
    const double* actual_first_conv_pos = GriffB10Common::firstVolIntersectionPos(trk_neutron,volname_conv);

    if (!actual_first_conv_pos) {
      ++c_skip_neutronmissconv;
      continue;
    }

    h_conv_vs_true->fill(Utils::dist(actual_first_conv_pos,true_conv_pos)/Units::mm);

    //Proceed if neutron converted:
    const double* actual_conv_pos = 0;
    if (trk_neutron->lastSegment()->volumeName()==volname_conv) {
      actual_conv_pos = trk_neutron->lastSegment()->lastStep()->postGlobalArray();
    }
    if (!actual_conv_pos)
      continue;

    if (dr.eventStorageMode()==GriffFormat::Format::MODE_FULL) {
      if (hit_full.eventHasHit())
        h_hit_vs_conv_full->fill(Utils::dist(hit_full.eventHitPosition(),actual_conv_pos)/Units::mm);
      if (hit_fullhighres.eventHasHit())
        h_hit_vs_conv_fullhighres->fill(Utils::dist(hit_fullhighres.eventHitPosition(),actual_conv_pos)/Units::mm);
      if (hit_red.eventHasHit()&&hit_fullhighres.eventHasHit())
        h_hit_redvsfullhighres->fill(Utils::dist(hit_fullhighres.eventHitPosition(),hit_red.eventHitPosition())/Units::mm);
    }
    if (hit_red.eventHasHit())
      h_hit_vs_conv_reduced->fill(Utils::dist(hit_red.eventHitPosition(),actual_conv_pos)/Units::mm);
    if (hit_redhighres.eventHasHit())
      h_hit_vs_conv_reducedhighres->fill(Utils::dist(hit_redhighres.eventHitPosition(),actual_conv_pos)/Units::mm);
    if (hit_std.eventHasHit())
      h_hit_vs_conv_std->fill(Utils::dist(hit_std.eventHitPosition(),actual_conv_pos)/Units::mm);

  }

  printf("Saving histograms to hitdbg.shist\n");
  hc.saveToFile("hitdbg",true);

  return 0;

}


