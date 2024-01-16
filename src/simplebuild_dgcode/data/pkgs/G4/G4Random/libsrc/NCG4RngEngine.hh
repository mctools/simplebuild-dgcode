#ifndef G4Random_NCG4RngEngine_hh
#define G4Random_NCG4RngEngine_hh

//Implementation of Hep Random Engine interface, wrapping the NCrystal
//Xoroshiro128+ RNG.

#include "CLHEP/Random/RandomEngine.h"
#include "NCrystal/internal/NCRandUtils.hh"

class NCG4RngEngine final : public CLHEP::HepRandomEngine  {
public:
  virtual ~NCG4RngEngine();
  std::string name() const override { return "NCrystalXoroshiroEngine"; }
  double flat() override { return doGenerate(); }
  void flatArray(const int size, double* vect) override
  {
    for ( int i = 0; i < size; ++ i )
      vect[i] = doGenerate();
  }
  //Direct access to RNG:
  void dgcode_set64BitSeed(std::uint64_t seed) { m_rng = NCrystal::RandXRSRImpl{ seed }; }
  std::uint64_t dgcode_genHighQuality64bitUint() { return m_rng.genUInt64(); }
  //Implementing the full interface just to be safe, but the following functions
  //are not really tested:
  void setSeed(long seed, int) override;
  void setSeeds(const long * seeds, int) override;
  void saveStatus( const char filename[] ) const override;
  void restoreStatus( const char filename[] ) override;
  void showStatus() const override;

  std::ostream& put( std::ostream& ) const override;
  std::istream& get( std::istream& ) override;

  std::vector<unsigned long> put () const override;
  bool get (const std::vector<unsigned long> &) override;

  bool getState (const std::vector<unsigned long> & v) override;
  std::istream & getState ( std::istream & is ) override;

private:
  double doGenerate() {
    //NCrystal rng shoots in (0,1] but we need to exclude 1 as well. We simply do:
    constexpr double lastvalbefore1 = 1.0 - std::numeric_limits<double>::epsilon();
    static_assert( lastvalbefore1 < 1.0, "");
    static_assert( lastvalbefore1 > 1.0-1e-15, "");
    return std::min<double>( m_rng.generate(), lastvalbefore1 );
  }
  NCrystal::RandXRSRImpl m_rng;
};

#endif
