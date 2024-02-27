#include "SimpleHists/Hist1D.hh"
#include "SimpleHists/Hist2D.hh"
#include "SimpleHists/HistCollection.hh"
#include <cassert>
#include <cmath>
#include <sstream>

namespace sh = SimpleHists;

namespace {
  class SimpleRandGen {
    // very simple multiply-with-carry rand gen (http://en.wikipedia.org/wiki/Random_number_generation)
  public:
    SimpleRandGen()
      : m_w(117),/* must not be zero, nor 0x464fffff */
        m_z(11713)/* must not be zero, nor 0x9068ffff */
    {
    }
    ~SimpleRandGen(){}

    double shoot()
    {
      m_w = 18000 * (m_w & 65535) + (m_w >> 16);
      m_z = 36969 * (m_z & 65535) + (m_z >> 16);
      return double((m_z << 16) + m_w)/double(UINT32_MAX);  /* 32-bit result */
    }
  private:
    std::uint32_t m_w;
    std::uint32_t m_z;
  };
}

int main(int,char**) {

  const int nhists=30;
  const int nperhist=60000;
  const int nbins=20;
  const double xmin=0;
  const double xmax=1;

  assert(nperhist%2==0);

  SimpleRandGen rand;

  sh::HistCollection hc1;//first half of vals
  sh::HistCollection hc2;//second half of vals
  sh::HistCollection hc12;//all vals

  for(int ihist=0;ihist<nhists;++ihist) {
    printf("Add histogram %i to test collections and fill\n",ihist);
    std::stringstream key;
    key << "hist_"<<ihist;
    auto h1  = hc1.book1D(key.str(),nbins,xmin,xmax,key.str());
    auto h2  = hc2.book1D(key.str(),nbins,xmin,xmax,key.str());
    auto h12 = hc12.book1D(key.str(),nbins,xmin,xmax,key.str());
    std::stringstream key2;
    key2 << "hist2d_"<<ihist;
    auto H1  = hc1.book2D(key2.str(),nbins,xmin,xmax,nbins,xmin,xmax,key2.str());
    auto H2  = hc2.book2D(key2.str(),nbins,xmin,xmax,nbins,xmin,xmax,key2.str());
    auto H12 = hc12.book2D(key2.str(),nbins,xmin,xmax,nbins,xmin,xmax,key2.str());
    std::stringstream key3;
    key3 << "histcounts_"<<ihist;
    auto hcount1  = hc1.bookCounts(key3.str(),key3.str());
    auto c1x = hcount1->addCounter("counter x");
    auto c1y = hcount1->addCounter("counter y");
    c1y.setComment("Some explanation about counter y goes here.");
    auto hcount2  = hc2.bookCounts(key3.str(),key3.str());
    auto c2x = hcount2->addCounter("counter x");
    auto c2y = hcount2->addCounter("counter y");
    c2y.setComment("Some explanation about counter y goes here.");
    auto hcount12 = hc12.bookCounts(key3.str(),key3.str());
    auto c12x = hcount12->addCounter("counter x");
    auto c12y = hcount12->addCounter("counter y");
    c12y.setComment("Some explanation about counter y goes here.");

    double weight(1);
    if (ihist>=5&&ihist<=8) weight=(ihist-5);
    else if (ihist!=2) weight *= 0.1;

    for (int i=0;i<nperhist;++i) {
      double val(sqrt(rand.shoot()*rand.shoot())*1.01-0.005);
      double yval(sqrt(rand.shoot()*rand.shoot())*1.01-0.005);
      if (ihist<10) {
        c12x += val;
        c12y += yval;
        if (i<nperhist/2) {
          c1x += val;
          c1y += yval;
        } else {
          c2x += val;
          c2y += yval;
        }
      } else if (ihist<20) {
        ++c12x;
        ++c12y;
        if (i<nperhist/2) {
          ++c1x;
          ++c1y;
        } else {
          ++c2x;
          ++c2y;
        }
      }
      if (ihist==15) {
        h12->fill(val);
        if (i<nperhist/2) h1->fill(val);
        else h2->fill(val);
        H12->fill(val,yval);
        if (i<nperhist/2) H1->fill(val,yval);
        else H2->fill(val,yval);
      } else {
        h12->fill(val,weight);
        if (i<nperhist/2) h1->fill(val,weight);
        else h2->fill(val,weight);
        H12->fill(val,yval,weight);
        if (i<nperhist/2) H1->fill(val,yval,weight);
        else H2->fill(val,yval,weight);
      }
    }

    printf("  .. performing basic serialisation/deserialisation/similarity checks\n");

    std::string s1; h1->serialise(s1);
    std::string s2; h2->serialise(s2);
    std::string s12; h12->serialise(s12);
    std::string S1; H1->serialise(S1);
    std::string S2; H2->serialise(S2);
    std::string S12; H12->serialise(S12);

    auto h1_clone = sh::deserialise(s1);//poor man's cloning
    auto h2_clone = sh::deserialise(s2);//poor man's cloning
    auto h12_clone = sh::deserialise(s12);//poor man's cloning
    auto H1_clone = sh::deserialise(S1);//poor man's cloning
    auto H2_clone = sh::deserialise(S2);//poor man's cloning
    auto H12_clone = sh::deserialise(S12);//poor man's cloning

    if (!h1_clone->isSimilar(h1) || !h2_clone->isSimilar(h2) || !h12_clone->isSimilar(h12)
        || !H1_clone->isSimilar(H1) || !H2_clone->isSimilar(H2) || !H12_clone->isSimilar(H12)) {
      printf("ERROR: serialised+deserialised histograms are not similar\n");
      return 1;
    }
  }

  printf("Attempting persistification to file and subsequent collection-level merging of re-loaded instances.\n");

  hc1.saveToFile("hc1");
  hc2.saveToFile("hc2.shist");
  hc12.saveToFile("hc12.shist");

  sh::HistCollection hca("hc12");
  sh::HistCollection hcb("hc1.shist");
  hcb.merge("hc2");

  if (!hca.isSimilar(&hcb)) {
    printf("ERROR: Merged collections are not similar\n");
    return 1;
  } else {
    printf("Success - merged collections are similar\n");
  }

  return 0;
}
