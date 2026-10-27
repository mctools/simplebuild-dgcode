#include "G4Interfaces/GeoConstructBase.hh"
#include <string>

namespace G4Tests {

  class GeoTest final : public G4Interfaces::GeoConstructBase {
  public:
    GeoTest() : GeoConstructBase("G4Tests/GeoTest")
    {
      addParameterDouble("boronThickness_micron",1.0,0.01,1000.0);
      addParameterDouble("worldExtent_meters",1.0,0.01,1000.0);
    }
    G4VPhysicalVolume* Construct() override;

    //Methods for testing extra methods in sbldpy modules (see testtopy):
    double worldVolume_m3() const
    {
      const double e = getParameterDouble("worldExtent_meters");
      return 16.0 * e * e * e;
    }
    std::string label( const std::string& prefix, int n, float x ) const
    {
      return prefix + ":" + std::to_string( n ) + ":" + std::to_string( x );
    }
    unsigned incrementCounter( unsigned step ) { return m_counter += step; }
  private:
    unsigned m_counter = 0;
  };

}
