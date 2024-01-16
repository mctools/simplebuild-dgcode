#ifndef G4Materials_NCG4Utils_hh
#define G4Materials_NCG4Utils_hh

class G4Material;
#include "NCrystal/NCMatCfg.hh"
#include <vector>

namespace NCG4Utils {

  //Create multi-phase alloys (really: weighted mixtures of crystallite). Note
  //that this function always creates a new G4Material when called, and does
  //*NOT* employ any caching behind the scenes. It is the callers responsibility
  //that the provided material name is unique for each call.
  typedef std::vector<std::pair<std::string,double> > AlloyList;
  typedef std::vector<std::pair<NCrystal::MatCfg,double> > AlloyListMatCfg;
  G4Material * createPCAlloy( const std::string& name, const AlloyList& nccfgs_and_weights, double density = 0 );
  G4Material * createPCAlloy( const std::string& name, const AlloyListMatCfg& nccfgs_and_weights, double density = 0 );

}

#endif
