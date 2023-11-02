#include "GriffDataRead/GriffDataReader.hh"
#include "GriffAnaUtils/All.hh"

#include "Core/FindData.hh"
#include "Units/Units.hh"

//Custom filter which selects just tracks which are
//daughters of a primary neutron:

class DaughterFilter: public GriffAnaUtils::ITrackFilter {

public:
  bool filter(const GriffDataRead::Track*trk) const {

    auto par = trk->getParent();
    if (!par) return false;
    if (!par->isPrimary()) return false;
    //2112 is a neutron, one could alternatively
    //check if par->pdgName() equals "neutron":
    return par->pdgCode() == 2112;
  }
};

int main(int,char**) {

  ///////////////////////////
  //// Setup data reader ////
  ///////////////////////////

  std::string datafile = Core::findData("GriffDataRead","10evts_singleneutron_on_b10_full.griff");
  GriffDataReader dr(datafile.c_str());

  //////////////////////////
  //// Setup selections ////
  //////////////////////////

  GriffAnaUtils::TrackIterator ti(&dr);

  ti.addFilter(new DaughterFilter);

  ////////////////////////////////////////////////////////
  //// Loop through selected data parts in all events ////
  ////////////////////////////////////////////////////////

  while (dr.loopEvents()) {
    printf("Found event %i/%i [ntrks=%i] ===========================================\n",
           dr.runNumber(),dr.eventNumber(),dr.nTracks());

    while (auto track =  ti.next()){

      printf("Neutron daughter info %s %s %f MeV \n",
             track->pdgNameCStr(),track->creatorProcessCStr(),track->startEKin()/Units::MeV);

    }

  }

  return 0;
}

