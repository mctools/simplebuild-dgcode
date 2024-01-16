#ifndef RandUtils_Rand_hh
#define RandUtils_Rand_hh

#include "NCrystal/internal/NCRandUtils.hh"

namespace RandUtils {

  class Rand {
  public:
    Rand(std::uint64_t seed = 123456789) : m_rng( seed ) {}
    ~Rand();

    double shoot() { return m_rng.generate(); }// uniformly in ]0,1]
    std::uint64_t shootUInt64() { return m_rng.genUInt64(); }//uniformly over 0..uint64max (i.e. all bits randomised)
    std::uint32_t shootUInt32() { return m_rng.genUInt32(); }//uniformly over 0..uint32max (i.e. all bits randomised)
    bool coinflip() { return m_rng.coinflip(); }

  private:
    NCrystal::RandXRSRImpl m_rng;
  };

}

#endif
