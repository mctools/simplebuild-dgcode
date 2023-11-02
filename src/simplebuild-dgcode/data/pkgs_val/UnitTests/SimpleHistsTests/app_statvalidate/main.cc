#include "SimpleHists/Hist1D.hh"
#include <cmath>

//Test numerical stability of RMS calculation in histograms. As a test-case, we
//consider a cluster of values (spread out O(width)) located far away from 0 (>>width).

namespace hist_stat {
  static const double width = exp(1);
  static const double weight = 1.1;
  static const double offset = width*1e6;
}
using namespace hist_stat;

void fill(SimpleHists::Hist1D&h,unsigned n) {
  assert(n%5==0);
  unsigned nn=n/5;
  for (unsigned i=0;i<nn;++i) {
    h.fill(1*width+offset,weight);
    h.fill(2*width+offset,weight);
    h.fill(3*width+offset,weight);
    h.fill(4*width+offset,weight);
    h.fill(5*width+offset,weight);
  }
}

int main(int,char**) {

  printf("\nCreating and filling test histogram:\n\n");

  const double expected_rms = sqrt(2)*width;
  const double expected_mean = 3*width+offset;

  SimpleHists::Hist1D hstattest("Stat test hist",100,0,offset+6*width);
  unsigned nprev=0;
  for (unsigned i=1;i<=7;++i) {
    unsigned n(pow(10,i));
    fill(hstattest,n-nprev);
    nprev=n;
    printf("  Mean relative deviation after %i fills : %g %%\n",nprev,100.0*(hstattest.getMean()-expected_mean)/expected_mean);
    printf("  RMS  relative deviation after %i fills : %g %%\n\n",nprev,100.0*(hstattest.getRMS()-expected_rms)/expected_rms);

  }

  return 0;
}
