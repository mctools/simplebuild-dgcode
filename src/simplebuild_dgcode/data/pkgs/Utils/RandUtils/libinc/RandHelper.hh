#ifndef RandUtils_RandHelper_hh
#define RandUtils_RandHelper_hh

#include <random>

namespace RandUtils {

  class RandHelper final {

    //Simple helper class wrapping C++11 RNG utilities. It provides a
    //deterministic stream of pseudo-random numbers from a Gaussian
    //distribution. If you need something seedable, just use the C++11 utilities
    //directly.

    //Usage example:
    // auto seed = 12345678;
    // RandHelper rng{seed};
    // for(int n=0; n<10; ++n) {
    //    std::cout<<rng.generate()<<std::endl;
    // for(int n=0; n<10; ++n) {
    //    std::cout<<rng.genNorm()<<std::endl;

  public:
    RandHelper()
      : m_engine(),
        m_randnorm(0.0,1.0),
        m_uniform01(0.0,1.0)
    {
    }
    template<typename ...Args>
    RandHelper( Args&& ...seedargs )
      : m_engine(),
        m_randnorm(0.0,1.0),
        m_uniform01(0.0,1.0)
    {
      std::seed_seq seed_sequence({std::forward<Args>(seedargs)...});
      m_engine.seed(seed_sequence);
    }
    ~RandHelper() = default;
    //Sample unit gaussian:
    double genNorm() { return m_randnorm(m_engine); }
    //Sample uniformly over (0.0,1.0]:
    double generate() { return 1.0-m_uniform01(m_engine); }//1.0-R to map [0,1) to (0,1].
  private:
    std::mt19937 m_engine;
    std::normal_distribution<> m_randnorm;
    std::uniform_real_distribution<> m_uniform01;
  };

}

#endif
