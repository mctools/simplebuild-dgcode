#include "SimpleHists/Hist1D.hh"
#include "SimpleHistsUtils/Sampler.hh"
#include "Core/Types.hh"
#include <cmath>

SimpleHists::Hist1D * createHist()
{
  auto h = new SimpleHists::Hist1D(20,-0.2,2.0);
  std::uint64_t n = 10000-1;
  for (std::uint64_t i = 0; i <= n; ++i) {
    double x = -log(double((i?i:1.0))/n);
    if (x>1.0&&x<1.5)
      x -= 5.0;
    if (x>5)
      x = 0.3;
    x-=0.05;
    h->fill(x);
  }
  return h;
}

void shortTests() {
  {
    printf("---- Only singular value in underflow:\n");
    SimpleHists::Hist1D h(100,-3,5);
    h.fill(-4.0);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Only values in underflow:\n");
    SimpleHists::Hist1D h(100,-3,5);
    h.fill(-4.0,17.0);
    h.fill(-3.1);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Only singular value in overflow:\n");
    SimpleHists::Hist1D h(100,-3,5);
    h.fill(6.0);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Only values in underflow:\n");
    SimpleHists::Hist1D h(100,-3,5);
    h.fill(5.5);
    h.fill(6,3.1);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Only singular value in one internal bin:\n");
    SimpleHists::Hist1D h(100,-3,5);//There is a bin from 2.6 to 2.68
    h.fill(2.65);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Only values in one internal bin:\n");
    SimpleHists::Hist1D h(100,-3,5);//There is a bin from 2.6 to 2.68
    h.fill(2.61);
    h.fill(2.67,15.1);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Only singular value at lower edge:\n");
    SimpleHists::Hist1D h(100,-3,5);//There is a bin from 2.6 to 2.68
    h.fill(-3);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Only singular value at upper edge:\n");
    SimpleHists::Hist1D h(100,-3,5);//There is a bin from 2.6 to 2.68
    h.fill(5);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Values in two adjacent internal bins:\n");
    SimpleHists::Hist1D h(100,-3,5);//There is a bin from 2.6-2.68, one from 2.68-2.76 and one from 2.76-2.84
    h.fill(2.64);
    h.fill(2.72);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Values in two adjacent internal bins, second more important:\n");
    SimpleHists::Hist1D h(100,-3,5);//There is a bin from 2.6-2.68, one from 2.68-2.76 and one from 2.76-2.84
    h.fill(2.64,1.0);
    h.fill(2.72,100.0);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Values in two non-adjacent internal bins:\n");
    SimpleHists::Hist1D h(100,-3,5);//There is a bin from 2.6-2.68, one from 2.68-2.76 and one from 2.76-2.84
    h.fill(2.64);
    h.fill(2.8);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Values in two non-adjacent internal bins, second more important:\n");
    SimpleHists::Hist1D h(100,-3,5);//There is a bin from 2.6-2.68, one from 2.68-2.76 and one from 2.76-2.84
    h.fill(2.64,1.0);
    h.fill(2.8,100.0);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Values in underflow and overflow only:\n");
    SimpleHists::Hist1D h(100,-3,5);//There is a bin from 2.6-2.68, one from 2.68-2.76 and one from 2.76-2.84
    h.fill(-4);
    h.fill(6);
    SimpleHists::Sampler s(&h);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
  {
    printf("---- Scaled values:\n");
    SimpleHists::Hist1D h(100,-3,5);
    h.fill(1.0);
    h.fill(2.0);
    SimpleHists::Sampler s(&h,10.0);
    printf("  Samples: %g %g %g %g %g %g\n",s(0.0),s(0.0001),s(0.4999),s(0.5001),s(0.9999),s(1.0));
  }
}

int main(int,char**) {

  shortTests();

  auto h_orig = createHist();
  h_orig->dump(true);

  SimpleHists::Sampler sampler(h_orig);
  auto h_sampled = new SimpleHists::Hist1D(100,-5,5.0);
  std::uint64_t n = 1000000-1;
  for (std::uint64_t i = 0; i <= n; ++i)
    h_sampled->fill(sampler(double(i)/n));

  h_sampled->dump(true);

  delete h_orig;
  delete h_sampled;

#if 0
  SimpleHists::HistCollection hc;
  hc.add(h_orig,"orig");
  hc.add(h_sampled,"sampled");
  hc.saveToFile("sample",true);
#endif

}
