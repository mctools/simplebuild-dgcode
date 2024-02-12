#include "GriffDataRead/GriffDataReader.hh"
#include "GriffAnaUtils/SegmentIterator.hh"
#include "GriffAnaUtils/SegmentFilter_Volume.hh"
#include "GriffAnaUtils/SegmentFilter_EnergyDeposition.hh"
#include "Utils/ArrayMath.hh"
#include "Utils/NeutronMath.hh"

#include "SimpleHists/HistCollection.hh"


int main (int argc,char**argv) {

  //////////////////////////////////////////////////
  //  Load data, extract setup and setup filters  //
  //////////////////////////////////////////////////

  GriffDataReader dr(argc,argv);
  auto setup = dr.setup();
  auto & geo = setup->geo();
  auto & gen = setup->gen();
  if (/*gen.getName()!="G4StdGenerators/ProfiledBeamGen"||*/geo.getName()!="G4GeoBoronTube/GeoBoronTube") {
    printf("Error: Wrong setup for this analysis\n");
    return 1;
  }

  setup->dump();

  const unsigned nbins=1000;

  //Filters:
  GriffAnaUtils::SegmentIterator segments_frontgas(&dr);
  segments_frontgas.addFilter(new GriffAnaUtils::SegmentFilter_Volume("CountingGas"));
  GriffAnaUtils::SegmentIterator segments_backgas(&dr);
  segments_backgas.addFilter(new GriffAnaUtils::SegmentFilter_Volume("InactiveCountingGas"));

  ///////////////////////
  //  Book histograms  //
  ///////////////////////

  SimpleHists::HistCollection hc;

  auto h_convhit_tubez_cm = hc.book1D("Converter hit position along tube [cm]",nbins,-50.0,50.0,"convhit_tubez_cm");//todo: use geo pars!

  auto h_qualitative = hc.bookCounts("Qualitative event descriptions","qualitative");
  auto c_all = h_qualitative->addCounter("All");//event count
  auto c_passed = h_qualitative->addCounter("passed");//neutron doesn't encounter anything before leaving the world
  auto c_wall_hit = h_qualitative->addCounter("wall_hit");//neutron passes the tube wall BEFORE converter
  auto c_wall_scat = h_qualitative->addCounter("wall_scat");//... and scatters in it
  auto c_wall_capt = h_qualitative->addCounter("wall_capt");//... and ends in it
  auto c_gas_scat = h_qualitative->addCounter("gas_scat");//neutron scatters in the (active) counting gas before converter
  auto c_endplug_hit = h_qualitative->addCounter("endplug_hit");//neutron passes an endplug before reaching the converter
  auto c_conv_hit = h_qualitative->addCounter("conv_hit");//neutron reaches the (front side) converter)
  auto c_conv_unscat = h_qualitative->addCounter("conv_unscat");//... and does so unscattered
  auto c_meas = h_qualitative->addCounter("meas");//edep in CountingGas > 120keV

  double emax_kev = 6000.0;

  //NB: The following histograms are NOT filled when there is an endplug hit
  //(if endplugs are simulated with Vacuum - for faster sim).
  const bool exclude_endplug_hits = (geo.getParameterString("material_endplugs")=="Vacuum");

  auto h_edep = hc.book1D("Energy deposited in counting gas [keV]",nbins,0.0,emax_kev,"edep");//this is the real thing
  auto h_edep_alphali = hc.book1D("Energy deposited in counting gas by Li/Alpha [keV]",nbins,0.0,emax_kev,"edep_alphali");
  auto h_edep_electrons = hc.book1D("Energy deposited in counting gas by electrons [keV]",nbins,0.0,emax_kev,"edep_electrons");
  auto h_edep_gammas = hc.book1D("Energy deposited in counting gas by gammas [keV]",nbins,0.0,emax_kev,"edep_gammas");
  auto h_edep_muons = hc.book1D("Energy deposited in counting gas by muons [keV]",nbins,0.0,emax_kev,"edep_muons");
  auto h_edep_back = hc.book1D("Energy deposited in backside counting gas [keV]",nbins,0.0,emax_kev,"edep_back");//to check the effect of the inactive back part
  auto h_edep_backfront = hc.book1D("Energy deposited in front or backside counting gas [keV]",nbins,0.0,emax_kev,"edep_backfront");//... same ...
  auto h_edep_noprescatter = hc.book1D("Energy deposited in counting gas for particles reaching the converter unscattered [keV]",nbins,0.0,emax_kev,"edep_noprescatter");
  auto h_wall_effect = hc.book1D("Kinetic energy of alpha/Li7 ions leaving counting gas [keV]",nbins,0.0,emax_kev,"wall_effect");

  double beamprof_cm(20);
  double ox(0);
  double oy(0);
  if (gen.getName()=="G4StdGenerators/ProfiledBeamGen") {
    double sx = gen.getParameterDouble("spread_x_mm")*0.1;
    double sy = gen.getParameterDouble("spread_y_mm")*0.1;
    ox = gen.getParameterDouble("offset_x_mm")*0.1;
    oy = gen.getParameterDouble("offset_y_mm")*0.1;
    if (gen.getParameterString("spread_mode")=="GAUSSIAN") {
      sx *= 3;
      sy *= 3;
    }
    beamprof_cm = 1.1 * std::max<double>(sx, sy);
    if (beamprof_cm < 0.1)
      beamprof_cm = 0.1;
  }
  auto h_beamprofile = hc.book2D("Beam profile in X-Y [cm]",
                                 200,ox-beamprof_cm,ox+beamprof_cm,
                                 200,oy-beamprof_cm,oy+beamprof_cm,
                                 "beamprofile");
  auto h_beamenergy = hc.book1D("Beam energy [Aangstrom]",
                                1000,0,30,"beamenergy");

  ////////////////////////////////////////////
  //  Loop over events and fill histograms  //
  ////////////////////////////////////////////


  while (dr.loopEvents()) {
    //Look at the primary neutron and figure out some qualitative properties of the event:
    auto trkPrimary = dr.primaryTrackBegin();
    assert(trkPrimary+1==dr.primaryTrackEnd()&&"event must contain exactly one primary particle");
    //assert(trkPrimary->pdgCode()==2112&&"primary particle must be a neutron");

    auto initial_pos = trkPrimary->firstSegment()->firstStep()->preGlobalArray();
    h_beamprofile->fill(initial_pos[0]/Units::cm,initial_pos[1]/Units::cm);

    if (trkPrimary->pdgCode()==2112) {
      h_beamenergy->fill(Utils::neutronEKinToWavelength(trkPrimary->startEKin())/Units::angstrom);
    }//todo: Make beam energy hist and use for all types
    assert(trkPrimary->firstSegment()->volumeName()=="World");
    auto initial_dir = trkPrimary->firstSegment()->firstStep()->preMomentumArray();

    bool hitEndPlug(false);
    bool hitFrontConverter(false);
    bool hitTubeWall(false);
    bool scatteredInTubeWall(false);
    bool endsInTubeWall(false);
    bool scatteredInGas(false);
    bool hitFrontConverterUnscattered(false);
    const double unscatted_cut_costh = 0.9999;
    for (auto nSeg = trkPrimary->segmentBegin();nSeg!=trkPrimary->segmentEnd();++nSeg) {
      if (!hitEndPlug&&nSeg->volumeName()=="EndPlug"&&!hitFrontConverter) hitEndPlug = true;
      if (hitEndPlug&&exclude_endplug_hits)
        continue;
      if (!hitTubeWall&&nSeg->volumeName()=="Detector"&&!hitFrontConverter) {
        hitTubeWall = true;
        if (nSeg == trkPrimary->lastSegment())
          endsInTubeWall = true;
        else if (Utils::costheta(nSeg->firstStep()->preMomentumArray(),
                                 nSeg->lastStep()->postMomentumArray())<=unscatted_cut_costh)
          scatteredInTubeWall = true;
      }
      if (!hitFrontConverter&&nSeg->volumeName()=="Converter"&&nSeg->volumeCopyNumber()>0) {
        hitFrontConverter = true;
        auto prevSeg = nSeg->getPreviousSegment();
        assert(prevSeg);
        if (prevSeg->volumeName()=="CountingGas") {
          h_convhit_tubez_cm->fill(prevSeg->lastStep()->postLocalZ()/Units::cm);
        }
        if (Utils::costheta(initial_dir,nSeg->firstStep()->preMomentumArray())>unscatted_cut_costh)
          hitFrontConverterUnscattered = true;
      }
      if (!scatteredInGas&&nSeg->volumeName()=="CountingGas") {
        if (Utils::costheta(nSeg->firstStep()->preMomentumArray(),nSeg->lastStep()->postMomentumArray())<=unscatted_cut_costh)
          scatteredInGas=true;
      }
    }

    ++c_all;
    if (trkPrimary->nSegments()==1) ++c_passed;
    if (hitTubeWall) ++c_wall_hit;
    if (scatteredInTubeWall) ++c_wall_scat;
    if (endsInTubeWall) ++c_wall_capt;
    if (scatteredInGas) ++c_gas_scat;
    if (hitEndPlug) ++c_endplug_hit;
    if (hitFrontConverter) ++c_conv_hit;
    if (hitFrontConverterUnscattered) ++c_conv_unscat;

    double edep_front = 0;
    double edep_front_alphalithium = 0;
    double edep_front_electrons = 0;
    double edep_front_muons = 0;
    double edep_front_gammas = 0;
    double wall_effect = 0;
    while (auto segment = segments_frontgas.next()) {
      if (exclude_endplug_hits&&hitEndPlug)
        continue;//ignore events where particle goes through endplug
                 //(in case endplug is simulated as Vacuum for efficiency)
      edep_front += segment->eDep();
      auto pdg = segment->getTrack()->pdgCode();
      if (pdg==1000020040/*alpha*/||pdg==1000030070/*Li7[0.0]*/) {
        wall_effect+=segment->endEKin();
        edep_front_alphalithium += segment->eDep();
      }
      if (pdg==11||pdg==-11) {
        edep_front_electrons += segment->eDep();
      }
      if (pdg==12||pdg==-12) {
        edep_front_muons += segment->eDep();
      }
      if (pdg==22||pdg==-22) {
        edep_front_gammas += segment->eDep();
      }
    }

    if (edep_front > 120*Units::keV)
      ++c_meas;

    double edep_back = 0;
    while (auto segment = segments_backgas.next()) {
      if (exclude_endplug_hits&&hitEndPlug)
        continue;//ignore events where particle goes through endplug (in case endplug is simulated as Vacuum for efficiency)
      edep_back += segment->eDep();
    }

    if (edep_front)
      h_edep->fill(edep_front/Units::keV);
    if (edep_front_alphalithium)
      h_edep_alphali->fill(edep_front_alphalithium/Units::keV);
    if (edep_front_electrons)
      h_edep_electrons->fill(edep_front_electrons/Units::keV);
    if (edep_front_muons)
      h_edep_muons->fill(edep_front_muons/Units::keV);
    if (edep_front_gammas)
      h_edep_gammas->fill(edep_front_gammas/Units::keV);
    if (hitFrontConverterUnscattered&&edep_front)
      h_edep_noprescatter->fill(edep_front/Units::keV);
    if (edep_back)
      h_edep_back->fill(edep_back/Units::keV);
    if (edep_back||edep_front)
      h_edep_backfront->fill((edep_front+edep_back)/Units::keV);
    if (wall_effect)
      h_wall_effect->fill(wall_effect/Units::keV);
  }


  hc.saveToFile("borontube");

  return 0;

}
