#ifndef G4Interfaces_GeoBase_hh
#define G4Interfaces_GeoBase_hh

class G4VPhysicalVolume;

#include "Utils/ParametersBase.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4Colour.hh"
class G4VSolid;
class G4Material;
//For convenience:
#include "Units/Units.hh"
#include "G4LogicalVolume.hh"
#include <cmath>

namespace G4Interfaces {

  class GeoBase : public Utils::ParametersBase
  {
  public:

    GeoBase(const char* name);
    virtual ~GeoBase();

    const char* getName() const { return m_name.c_str(); }

    void dump(const char * prefix="") const override;


    //Convenience method which feeds the value of a string constructs a material from the value of a string
    //parameter via the NamedMaterialProvider (the material can therefore be changed by users):
    G4Material * getParameterMaterial(const std::string& name_of_string_parameter) const;

    //Convenience method which constructs a material from a string via the
    //NamedMaterialProvider (the material is thus likely hardcoded with the
    //geometry):
    G4Material * getMaterial(const std::string& string_for_named_material_provider) const;

  protected:

    //By default volumes are checked for overlaps when place(..)'ed (see
    //below). This can be slow for complex geometries, so can be toggled on/off
    //by calls to these methods:
    void disableOverlapChecking() { m_checkOverlaps = false; }
    void enableOverlapChecking() { m_checkOverlaps = true; }

    //Convenience function for easily placing shapes in derived classes:

    struct PlacedVols
    {
      G4LogicalVolume* logvol;
      G4PVPlacement* physvol;
    };

    //Directly from shape to physical volume (translating with [x,y,z] and optionally rotating with rot):
    PlacedVols place(G4VSolid* shape,G4Material*mat,double x=0.0,double y=0.0,double z=0.0,
                     G4LogicalVolume*mother=nullptr,const G4Colour&col = WHITE,
                     G4int copyNum=0, const char * name = 0,
                     G4RotationMatrix* rot = 0);

    //Same, but accepting a transformation object:
    PlacedVols place(G4VSolid* shape,G4Material*mat,const G4Transform3D&,
                     G4LogicalVolume*mother=nullptr,const G4Colour&col = WHITE,
                     G4int copyNum=0, const char * name = 0);

    //From logical volume (will override the logical volume colour as a side-effect):
    PlacedVols place(G4LogicalVolume* lv,double x=0.0,double y=0.0,double z=0.0,
                     G4LogicalVolume*mother=nullptr,const G4Colour&col = WHITE,
                     G4int copyNum=0, const char * name = 0,
                     G4RotationMatrix* rot = 0);

    PlacedVols place(G4LogicalVolume* l,const G4Transform3D&,
                     G4LogicalVolume*mother=nullptr,const G4Colour&col = WHITE,
                     G4int copyNum=0, const char * name = 0);

    //Some pre-defined colours:
    static const G4Colour SILVER;
    static const G4Colour GOLD;

    static const G4Colour WHITE;
    static const G4Colour GRAY;
    static const G4Colour BLACK;

    static const G4Colour RED;
    static const G4Colour BLUE;
    static const G4Colour GREEN;
    static const G4Colour YELLOW;
    static const G4Colour ORANGE;
    static const G4Colour CYAN;
    static const G4Colour PURPLE;
    static const G4Colour BROWN;

    static const G4Colour DARKRED;
    static const G4Colour DARKBLUE;
    static const G4Colour DARKGREEN;
    static const G4Colour DARKYELLOW;
    static const G4Colour DARKORANGE;
    static const G4Colour DARKCYAN;
    static const G4Colour DARKPURPLE;
    static const G4Colour DARKBROWN;

    //html 4.01 names as well (note that html 4.01 PURPLE is our DARKPURPLE)
    static const G4Colour MAROON;// same as DARKRED
    static const G4Colour LIME;// same as GREEN
    static const G4Colour OLIVE;//same as DARKYELLOW
    static const G4Colour NAVY;//same as DARKBLUE
    static const G4Colour AQUA;//same as CYAN
    static const G4Colour TEAL;//same as DARKCYAN
    static const G4Colour FUCHSIA;//same as PURPLE
    static const G4Colour MAGENTA;//same as PURPLE
    static const G4Colour INVISIBLE;//Special colour, only for G4 viewer.

  private:
    std::string m_name;
    bool m_checkOverlaps;
  };

}

#endif
