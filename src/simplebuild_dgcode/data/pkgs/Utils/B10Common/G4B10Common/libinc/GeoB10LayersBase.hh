#ifndef G4B10Common_GeoB10LayersBase_hh
#define G4B10Common_GeoB10LayersBase_hh

#include "G4B10Common/GeoB10Base.hh"

//Base class for the various B4C detectors utilising a concept of layers and
//cells. It provides a few common parameters and a utility for consistent
//copynumber assignment.
//
//The detector should be placed so a gun at (0,0,0) firing at (0,0,1) will hit it.

class GeoB10LayersBase : public GeoB10Base
{
public:
  GeoB10LayersBase(const char* name);
  virtual ~GeoB10LayersBase();

protected:
  //If derived class implemented validateParameters, it should call this
  //implementation from within it:
  virtual bool validateParameters();

  //Copy number assignment for layers/cells. It is constructed so in analysis
  //code the info can be unpacked with:
  //   layernumber: copynumber % 10000;
  //   cellnumber:  copynumber / 10000;
  //
  //Note that if the volume is inside a logical volume which is shared, then
  //simply set the copynumber to -1 and set the proper copynumber above when
  //placing the mother logical volume. That way, analysis code can take the
  //copy number if it is >=0, and go one (N) level(s) up and look for it there
  //instead it if it is -1 (-N).
  G4int copyNumber(G4int layernumber,G4int cellnumber = 0);
};


#endif
