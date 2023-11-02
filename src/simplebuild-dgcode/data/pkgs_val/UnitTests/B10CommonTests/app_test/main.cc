#include "GriffB10Common/DetHitApproximation.hh"
#include "GriffDataRead/GriffDataReader.hh"
#include "Core/FindData.hh"
#include "Core/FPE.hh"

int main(int,char**) {
  //Bonus test:
  Core::catch_fpe();

  //Input file doesn't matter, we just want to instantiate DetHitApproximation
  //correctly and then trigger a call to DetHitApproximation::test():
  GriffDataReader dr(Core::findData("GriffDataRead","10evts_singleneutron_on_b10_full.griff"));
  DetHitApproximation hitappr(&dr);

  //Trigger tests (actual test is comparison with reference output):
  hitappr.test();

  return 0;
}

