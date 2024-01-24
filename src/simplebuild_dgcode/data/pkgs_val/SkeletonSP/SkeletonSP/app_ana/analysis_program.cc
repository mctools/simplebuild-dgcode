#include "GriffAnaUtils/All.hh"
#include "Units/Units.hh"
#include "Utils/ArrayMath.hh"
#include "SimpleHists/HistCollection.hh"

//<SKEL_MUST_MODIFY_FILE>

//Griff analysis. See https://mctools.github.io/simplebuild-dgcode/griff.html
//for more info.

int main(int argc,char**argv) {

  //Open .griff file(s) specified on the command line:
  GriffDataReader dr(argc,argv);

  //Extract and dump meta-data:
  dr.setup()->dump();

  //Check (as an example) that the simulated geometry is of the correct type:
  if (dr.setup()->geo().getName()!="G4GeoSkeletonSP/GeoSkeletonSP")
    return 1;

  //Book histograms (find more info at
  //https://mctools.github.io/simplebuild-dgcode/simplehists.html):

  SimpleHists::HistCollection hc;

  double dxy_mm = 0.6*dr.setup()->geo().getParameterDouble("detector_size_cm")*10;
  auto h2d_box_hitmap = hc.book2D("Hit position when entering detector",
                                   40,-dxy_mm,dxy_mm,40,-dxy_mm,dxy_mm,"det_hitmap");
  h2d_box_hitmap->setComment("This is the xy coordinates of primary particles "
                             "as they enter the detector volume.");
  h2d_box_hitmap->setXLabel("mm");
  h2d_box_hitmap->setYLabel("mm");

  auto h1d_box_hitradius = hc.book1D("Hit radial position when entering detector",
                                     60, 0, dxy_mm, "det_hitradius");
  h1d_box_hitradius->setComment("This is the radial coordinates of primary "
                                "particles as they enter the detector volume.");
  h1d_box_hitradius->setXLabel("mm");

  //Loop over events and extract info via Griff interface:
  while (dr.loopEvents()) {
    for (auto trk = dr.primaryTrackBegin();trk!=dr.primaryTrackEnd();++trk) {
      for (auto seg = trk->segmentBegin(); seg!=trk->segmentEnd(); ++seg) {
        if (seg->volumeName()=="Detector" && seg->endAtVolumeBoundary()) {
          auto s = seg->firstStep();
          double x(s->preLocalX()/Units::mm),y(s->preLocalY()/Units::mm);
          if (x*x+y*y>0.01) {
            h2d_box_hitmap->fill(x,y);
            h1d_box_hitradius->fill(sqrt(x*x+y*y));
          }
        }
      }
    }
  }

  //Save histograms to a file which can be browsed with sb_simplehists_browse:
  hc.saveToFile("skeletonsp",true);

  return 0;
}


