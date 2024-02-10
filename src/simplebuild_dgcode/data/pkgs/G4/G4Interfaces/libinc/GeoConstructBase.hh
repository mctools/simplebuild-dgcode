#ifndef G4Interfaces_GeoConstructBase_hh
#define G4Interfaces_GeoConstructBase_hh

class G4VPhysicalVolume;

#include "G4Interfaces/GeoBase.hh"
#include "G4VUserDetectorConstruction.hh"

namespace G4Interfaces {

  //NB: Very important for python export that first listed base-class is GeoBase!

  class GeoConstructBase : public GeoBase, public G4VUserDetectorConstruction {
  public:
    //Derived classes must:
    //
    //  1) Provide a name with  : GeoConstructBase("some name")
    //  2) Use the ParametersBase addParameterXXX calls in the constructor to
    //     define free parameters which the detector construction will depend on.
    //  3) Implement detector construction in (using getParameterXXX calls):
    //        G4VPhysicalVolume* Construct();

    GeoConstructBase(const char* name);
    virtual ~GeoConstructBase();

    virtual G4VPhysicalVolume* Construct() = 0;

  };


}

#endif
