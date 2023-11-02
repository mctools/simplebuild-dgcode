#include "GriffAnaUtils/StepIterator.hh"
#include "GriffAnaUtils/SegmentIterator.hh"
#include "GriffAnaUtils/TrackIterator.hh"

#include "GriffAnaUtils/TrackFilter_PDGCode.hh"
#include "GriffAnaUtils/SegmentFilter_EnergyDeposition.hh"
#include "GriffAnaUtils/SegmentFilter_Volume.hh"
#include "GriffAnaUtils/StepFilter_EnergyDeposition.hh"

#include "GriffDataRead/GriffDataReader.hh"
#include "Core/FindData.hh"
#include "Units/Units.hh"
#include "Core/FPE.hh"
#include <iostream>

int main(int,char**) {

  Core::catch_fpe();

  ///////////////////////////
  //// Setup data reader ////
  ///////////////////////////

  std::string datafile = Core::findData("GriffDataRead","10evts_singleneutron_on_b10_full.griff");
  GriffDataReader dr(datafile.c_str());

  //////////////////////////
  //// Setup selections ////
  //////////////////////////

  //Setup selection of segments:
  GriffAnaUtils::SegmentIterator si(&dr);

  //Ignore photons:
  si.addFilter(new GriffAnaUtils::TrackFilter_PDGCode(22))->setNegated();

  //Only segments with significant energy deposit:
  si.addFilter(new GriffAnaUtils::SegmentFilter_EnergyDeposition(1*Units::keV));

  //particles outside target box only:
  GriffAnaUtils::SegmentIterator si2(&dr);
  si2.addFilter(new GriffAnaUtils::SegmentFilter_Volume("lv_targetbox"))->setNegated();

  GriffAnaUtils::TrackIterator ti(&dr);

  //Only alpha and Li tracks:
  auto tf_alpha_lithium = (new GriffAnaUtils::TrackFilter_PDGCode(1000020040,1000030070))->setUnsigned();
  ti.addFilter(tf_alpha_lithium);

  //Only steps in lv_targetbox with large energy deposit and from alphas or lithium:
  GriffAnaUtils::StepIterator stepit(&dr);
  stepit.addFilter(new GriffAnaUtils::StepFilter_EnergyDeposition(50.0*Units::keV));
  stepit.addFilter(new GriffAnaUtils::SegmentFilter_Volume("lv_targetbox"));
  stepit.addFilter(tf_alpha_lithium);

  ////////////////////////////////////////////////////////
  //// Loop through selected data parts in all events ////
  ////////////////////////////////////////////////////////

  while (dr.loopEvents()) {
    std::cout<<"Found event "<<dr.runNumber()<<"/"<<dr.eventNumber()<<" [ntrks="<<dr.nTracks()<<", seed="<<dr.seed()
             <<"] ==========================================="<<std::endl;
    while (auto segment =  si.next()) {
      printf("  Found segment %i (edep=%.1fkeV in %s) on track %i (pdg=%i)\n",
             segment->iSegment(),
             segment->eDep()/Units::keV,segment->volumeName().c_str(),
             segment->getTrack()->trackID(),segment->getTrack()->pdgCode());
    }
    while (auto track =  ti.next())
      printf("  Loop over all alpha/lithium tracks: track %i (%s)\n",track->trackID(),track->pdgNameCStr());

    while (auto segment =  si2.next()) {
      printf("  Found segment outside target: %i (%s in %s)\n",
             segment->iSegment(),
             segment->getTrack()->pdgNameCStr(),segment->volumeNameCStr());
    }

    while (auto step =  stepit.next()) {
      printf("  Found step %i (edep=%.1fkeV in %s) on track %i (%s) segment %i\n",
             step->iStep(),step->eDep()/Units::keV,step->getSegment()->volumeNameCStr(),
             step->getSegment()->getTrack()->trackID(),
             step->getSegment()->getTrack()->pdgNameCStr(),
             step->getSegment()->iSegment());
    }

  }
  return 0;
}

