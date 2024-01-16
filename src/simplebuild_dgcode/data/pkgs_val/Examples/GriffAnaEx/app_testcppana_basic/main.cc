#include "GriffDataRead/GriffDataReader.hh"
#include "Core/FindData.hh"
#include "Units/Units.hh"

int main(int,char**) {

  //A very simple analysis where we avoid using any iterators or filters from
  //GriffAnaUtils. Rather we simply go through the tracks in each event and use
  //them to get to their segments and steps and extract information.
  //
  //The analysis here is not meant to be particularly meaningful from a physics
  //point of view. Additionally, one would often accumulate data in histograms
  //for further processing, rather than just calculating and printing some info
  //like we do here. Also, for large data-sets, one would probably not have a
  //lot of printouts inside the event loop.

  //Open file:
  std::string datafile = Core::findData("GriffDataRead","10evts_singleneutron_on_b10_full.griff");
  GriffDataReader dr(datafile.c_str());

  //Show how we can access meta-data:
  printf("Loading file with boron thickness of %g micron\n",
         dr.setup()->geo().getParameterDouble("boronThickness_micron"));

  //Loop over the events:
  while (dr.loopEvents()) {

    printf("Reading event %i/%i [ntrks=%i] ===========================================\n",
           dr.runNumber(),dr.eventNumber(),dr.nTracks());

    double edep = 0;

    //Loop over all tracks (primaries and secondaries) and find energy deposits
    //inside volumes with a given name:

    for (auto trk = dr.trackBegin();trk!=dr.trackEnd();++trk) {
      for (auto seg = trk->segmentBegin(); seg!=trk->segmentEnd(); ++seg) {
        if (seg->volumeName()=="lv_targetbox") {
          //segment already contains summed step info such as energy deposition:
          if (seg->eDep()>0) {
            edep += seg->eDep();
            //only if we need stuff like coordinates do we normally need to
            //access the steps, and very often it is enough to use the first or
            //last step rather than looping through all of them:
            printf("  segment of %s with edep starts at z=%g mm\n",
                   trk->pdgNameCStr(),
                   seg->firstStep()->preGlobalZ()/Units::mm);
          }
        }
      }
    }

    printf("Event had a total edep in the targetbox of %g MeV\n",edep/Units::MeV);
  }

  return 0;
}
