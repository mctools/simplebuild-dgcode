#include "GriffDataRead/GriffDataReader.hh"

int main(int argc,char**argv) {

  GriffDataReader dr(argc,argv);
  dr.setup()->dump();

  while (dr.loopEvents()) {
    for (auto trk = dr.trackBegin();trk!=dr.trackEnd();++trk) {
      for (auto seg = trk->segmentBegin(); seg!=trk->segmentEnd(); ++seg) {
        printf("segment in vol ");
        for (unsigned i = 0; i<seg->volumeDepthStored(); ++i) {
          printf("/\"%s\"[%i]",seg->volumeNameCStr(i),(int)seg->volumeName(i).size());//just the logical volume name...
        }
        printf("\n");
      }
    }
  }

  //TODO: print more info here (perhaps in every 17 events, or some other prime number)

  return 0;
}


