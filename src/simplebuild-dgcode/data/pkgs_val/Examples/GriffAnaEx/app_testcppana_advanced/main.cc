#include "GriffDataRead/GriffDataReader.hh"
#include "Core/FindData.hh"
#include "Units/Units.hh"

#include "GriffAnaUtils/All.hh"

int main(int,char**) {

  //Same analysis as in the cppana_basic application in this package (see
  //comments in there). But this time using the somewhat more handy and
  //efficient iterators and filters to fish our the information we want from the
  //event.

  //Open file:
  std::string datafile = Core::findData("GriffDataRead","10evts_singleneutron_on_b10_full.griff");
  GriffDataReader dr(datafile.c_str());

  //Show how we can access meta-data:
  printf("Loading file with boron thickness of %g micron\n",
         dr.setup()->geo().getParameterDouble("boronThickness_micron"));

  //Setup iterators and filters:
  GriffAnaUtils::SegmentIterator si(&dr);
  si.addFilter(new GriffAnaUtils::SegmentFilter_Volume("lv_targetbox"));
  si.addFilter(new GriffAnaUtils::SegmentFilter_EnergyDeposition(0.0));

  //Loop over the events:
  while (dr.loopEvents()) {

    printf("Reading event %i/%i [ntrks=%i] ===========================================\n",
           dr.runNumber(),dr.eventNumber(),dr.nTracks());

    double edep = 0;

    while (auto seg = si.next()) {
      edep += seg->eDep();
      printf("  segment of %s with edep starts at z=%g mm\n",
             seg->getTrack()->pdgNameCStr(),
             seg->firstStep()->preGlobalZ()/Units::mm);
    }

    printf("Event had a total edep in the targetbox of %g MeV\n",edep/Units::MeV);
  }

  return 0;
}
