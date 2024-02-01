#ifndef G4B10Common_GeoB10Base_hh
#define G4B10Common_GeoB10Base_hh

#include "G4Interfaces/GeoConstructBase.hh"

//commonly includes for derived classes here:
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4ThreeVector.hh"

//Base class for detectors with B4C converters and a counting gas. Provides
//materials, names and colours in order to avoid work duplication and achieve a
//higher level of consistency in each specific detector geometry description as
//well as a consistent interface.

class GeoB10Base : public G4Interfaces::GeoConstructBase
{
public:
  GeoB10Base(const char* name);
  virtual ~GeoB10Base();

protected:
  //If derived class implements validateParameters, it should call this
  //implementation from within it:
  virtual bool validateParameters();

  //For consistency, derived classes should use the following methods during
  //geometry construction for materials, names and colours (it is safe to call
  //the methods more than once as they cache any created objects behind the
  //scenes):

  G4Material * createMaterialConverter() const;
  G4Material * createMaterialCountingGas() const;
  G4Material * createMaterialSubstrate() const;
  G4Material * createMaterialDetectorBox() const;
  G4Material * createMaterialWorld() const;

  const char * physVolName_World() const { return "World"; }
  const char * physVolName_Converter() const { return "Converter"; }
  const char * physVolName_CountingGas() const { return "CountingGas"; }
  const char * physVolName_InactiveCountingGas() const { return "InactiveCountingGas"; }
  const char * physVolName_Substrate() const { return "Substrate"; }
  const char * physVolName_DetectorBox() const { return "Detector"; }

  double colour_alpha() const;
  G4Colour colour_World() const;
  G4Colour colour_Converter() const;
  G4Colour colour_CountingGas() const;
  G4Colour colour_InactiveCountingGas() const;
  G4Colour colour_Substrate() const;
  G4Colour colour_DetectorBox() const;
};

#endif
