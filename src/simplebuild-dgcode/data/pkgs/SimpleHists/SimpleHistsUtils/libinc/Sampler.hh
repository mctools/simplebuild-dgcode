#ifndef SimpleHists_Sampler_hh
#define SimpleHists_Sampler_hh

#include <vector>

namespace SimpleHists {

  class Hist1D;

  class Sampler {

    //Initialise (or reinit) from a 1D histogram in order to be able to sample
    //the contained distribution at a later stage, by providing random numbers
    //uniformly distributed between 0 and 1. Optionally supply a scale-factor to
    //sampled values (making it easy to incorporate unit conversions between
    //histogram bin units and needed unit of sampled quantities).

  public:
    Sampler(Hist1D*, double scalefact = 1.0 );
    ~Sampler();

    double sample(double rand) const;
    double operator()(double rand) const { return sample(rand); }

    void reinit(Hist1D*, double scalefact = 1.0);

  private:
    std::vector<double> m_cumul;
    std::vector<double> m_edges;
  };
}


#endif
