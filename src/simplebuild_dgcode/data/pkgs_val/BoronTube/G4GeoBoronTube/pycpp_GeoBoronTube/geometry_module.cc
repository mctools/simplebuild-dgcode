
/////////////////////////////////////////
// Declaration of our geometry module: //
/////////////////////////////////////////

#include "G4Interfaces/GeoConstructPyExport.hh"
#include "G4B10Common/GeoB10Base.hh"

namespace {
  //Using GeoB10Base as base class, to get some common utilities for B10-based
  //detectors:
  class GeoBoronTube final : public GeoB10Base {
  public:
    GeoBoronTube();
    G4VPhysicalVolume* Construct() override;
    //(add more member functions and data here if needed)
  protected:
    bool validateParameters() override;

  };
}

PYTHON_MODULE( mod )
{
  GeoConstructPyExport::exportGeo<GeoBoronTube>(mod, "GeoBoronTube");
}

////////////////////////////////////////////
// Implementation of our geometry module: //
////////////////////////////////////////////

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4IntersectionSolid.hh"
#include "G4Transform3D.hh"
#include "G4Vector3D.hh"

//As an example, we put a spherical sample in front of a box representing a
//detector tube (using the Griff analysis to look for positions where neutrons
//enter the detector).

GeoBoronTube::GeoBoronTube()
  : GeoB10Base("G4GeoBoronTube/GeoBoronTube")
{
  //vis_phimax_deg is for opening up the geometry for screenshots:
  addParameterInt("vis_phimax_deg", 360, 1, 360);
  addParameterBoolean("vis_remove_ends", false);
  addParameterString("material_endplugs", "stdlib::Nylon12_C12H23NO.ncmat");
  //detbox_length_cm must be at least twice the blade length:
  addParameterDouble("detbox_length_cm", 50, 0.01, 1000.0 );
  addParameterDouble("detbox_radius_mm", 25.5, 0.01, 1000.0 );
  addParameterDouble("detbox_wall_mm", 1.5, 0.01, 20.0);
  addParameterDouble("substrate_length_cm", 18, 0.01, 1000.0 );
  addParameterDouble("substrate_width_cm", 2, 0.01, 1000.0 );
  addParameterDouble("substrate_thickness_mm", 0.5, 0.01, 1000.0 );
  addParameterDouble("endplug_thickness_cm", 3, 0.01, 1000.0 );
  addParameterDouble("substrate_endplug_dist_cm", 2, 0.01, 1000.0 );
  addParameterDouble("converter_micron", 1, 0.1, 1001.0);
  addParameterDouble("incidence_angle_deg", 90, 0.01, 179.9 );
  addParameterDouble("incidence_position_cm", 0, -1000.0, 1000.0 );
  //set >0 to install a fake B10 filled volume X mm inside the endplugs (to
  //reduce needless CPU consumption):
  addParameterDouble("fake_endplug_hole_depth_mm", 0, 0, 1000.0 );
}

bool GeoBoronTube::validateParameters()
{
  if (!GeoB10Base::validateParameters())
    return false;

  //TODO: endplug and blade dimensions (with converter) must fit inside
  return true;
}

G4VPhysicalVolume* GeoBoronTube::Construct()
{
  auto matGas = createMaterialCountingGas();
  auto matSubstrate = createMaterialSubstrate();
  auto matDetBox = createMaterialDetectorBox();
  auto matConverter = createMaterialConverter();
  auto matWorld = createMaterialWorld();
  auto matEndPlugs = getParameterMaterial("material_endplugs");

  const char * name_inactive_gas = physVolName_InactiveCountingGas();
  const char * name_detbox = physVolName_DetectorBox();
  const char * name_substrate = physVolName_Substrate();
  const char * name_gas = physVolName_CountingGas();
  const char * name_converter = physVolName_Converter();
  const char * name_endplug = "EndPlug";
  const char * name_endplug_hole = "FakeEndPlugHole";

  const double phimax = ( getParameterInt("vis_phimax_deg")==360
                          ? 2*M_PI
                          : getParameterInt("vis_phimax_deg")*M_PI/180.0 );
  const bool remove_ends = getParameterBoolean("vis_remove_ends");
  const double detbox_length = getParameterDouble("detbox_length_cm")*Units::cm;
  const double detbox_inner_radius = getParameterDouble("detbox_radius_mm")*Units::mm;
  const double detbox_wall = getParameterDouble("detbox_wall_mm")*Units::mm;
  const double detbox_outer_radius = detbox_inner_radius + detbox_wall;
  const double substrate_length = getParameterDouble("substrate_length_cm")*Units::cm;
  const double substrate_width = getParameterDouble("substrate_width_cm")*Units::cm;
  const double substrate_thickness = getParameterDouble("substrate_thickness_mm")*Units::mm;
  const double converter_thickness = getParameterDouble("converter_micron")*Units::um;

  const double endplug_thickness = getParameterDouble("endplug_thickness_cm")*Units::cm;
  const double substrate_endplug_dist = getParameterDouble("substrate_endplug_dist_cm")*Units::cm;

  const double fake_endplug_hole_depth = getParameterDouble("fake_endplug_hole_depth_mm")*Units::mm;

  const double incidence_angle = getParameterDouble("incidence_angle_deg")*Units::deg;
  const double incidence_pos = getParameterDouble("incidence_position_cm")*Units::cm;

  double big_dimension = 1.1*(detbox_length+2*detbox_outer_radius+std::abs<double>(incidence_pos));

  G4Box* solidWorld = new G4Box(physVolName_World(), 2*big_dimension,2*big_dimension,2*big_dimension);
  auto worldvols = place(solidWorld,matWorld,0,0,0,0,colour_World());
  auto lvWorld = worldvols.logvol;
  auto pvWorld = worldvols.physvol;

  auto col_detbox = colour_DetectorBox();
  auto col_gas = colour_CountingGas();
  auto col_gasinact = colour_InactiveCountingGas();
  auto col_substrate = colour_Substrate();
  auto col_converter = colour_Converter();
  auto col_endplug = G4Colour(0.5,0.5,0.2,colour_alpha());
  auto col_endplug_hole = G4Colour(1,0,0,colour_alpha());

  /////////////////////////////////////////
  // Det tube, counting gas and endplugs //
  /////////////////////////////////////////

  const double a = std::sqrt( detbox_inner_radius*detbox_inner_radius
                              -0.25*substrate_width*substrate_width );

  G4Transform3D trf(  G4Translate3D(0,0,big_dimension)
                      * G4Rotate3D(90*Units::deg, G4Vector3D(0,0,1))
                      * G4Rotate3D(180*Units::deg, G4Vector3D(1,0,0))
                      * G4Rotate3D(-incidence_angle, G4Vector3D(0,1,0))
                      * G4Translate3D( (a-substrate_thickness-2*converter_thickness),
                                       0,incidence_pos)
                      );

  auto lvDetBox = place(new G4Tubs( name_detbox,0,detbox_outer_radius,
                                    0.5*detbox_length,0.0,phimax),
                        matDetBox,trf,lvWorld,col_detbox,0,0).logvol;

  if (!remove_ends) {
    auto lvEndPlug = new G4LogicalVolume(new G4Tubs( name_endplug,0,
                                                     detbox_inner_radius,
                                                     0.5*endplug_thickness,
                                                     0.0,phimax),
                                         matEndPlugs,name_endplug);
    if (fake_endplug_hole_depth) {
      place(new G4Tubs( name_endplug_hole,0,detbox_inner_radius-fake_endplug_hole_depth,
                        0.5*endplug_thickness-fake_endplug_hole_depth,0.0,phimax),
            matWorld,0,0,0,lvEndPlug,col_endplug_hole,0,0);
    }

    place(lvEndPlug,0,0,-0.5*(detbox_length-endplug_thickness),lvDetBox,col_endplug,-1);
    place(lvEndPlug,0,0,0.5*(detbox_length-endplug_thickness),lvDetBox,col_endplug,1);

  }

  auto shape_gas = new G4Tubs( name_gas,0,detbox_inner_radius,
                               0.5*detbox_length-endplug_thickness,
                               0.0,phimax);
  auto lvGas = place(shape_gas,matGas,0,0,0,lvDetBox,col_gas).logvol;


  ////////////
  // Blades //
  ////////////

  auto shape_substrate = new G4Box(name_substrate,0.5*substrate_thickness,0.5*substrate_width,0.5*substrate_length);
  auto shape_converter = new G4Box(name_converter,0.5*converter_thickness,0.5*substrate_width,0.5*substrate_length);

  double offset = a;
  const double z3 = 0.5*detbox_length-endplug_thickness-substrate_endplug_dist-0.5*substrate_length;
  const double z1 = -z3;

  place(shape_converter,matConverter,-(offset-0.5*converter_thickness),0,z1,lvGas,col_converter,-1);
  place(shape_converter,matConverter,-(offset-0.5*converter_thickness),0,z3,lvGas,col_converter,-3);
  offset -= converter_thickness;

  place(shape_substrate,matSubstrate,-(offset-0.5*substrate_thickness),0,z1,lvGas,col_substrate,1);
  place(shape_substrate,matSubstrate,-(offset-0.5*substrate_thickness),0,z3,lvGas,col_substrate,3);
  offset -= substrate_thickness;

  place(shape_converter,matConverter,-(offset-0.5*converter_thickness),0,z1,lvGas,col_converter,1);
  place(shape_converter,matConverter,-(offset-0.5*converter_thickness),0,z3,lvGas,col_converter,3);
  offset -= converter_thickness;

  place(shape_converter,matConverter,-(offset-0.5*converter_thickness),0,0,lvGas,col_converter,-2);
  offset -= converter_thickness;

  place(shape_substrate,matSubstrate,-(offset-0.5*substrate_thickness),0,0,lvGas,col_substrate,2);
  offset -= substrate_thickness;

  place(shape_converter,matConverter,-(offset-0.5*converter_thickness),0,0,lvGas,col_converter,2);
  offset -= converter_thickness;

  //////////////////
  // Inactive Gas //
  //////////////////

  //Add inactive gas volumes on the outside of the substrates, as we surely can not collect charges from there.

  //First a box for the central part:
  const double inactive_gas_length0 = detbox_length-2*endplug_thickness-2*substrate_endplug_dist-2*substrate_length;
  assert(inactive_gas_length0>0);
  auto shape_inact0 = new G4Box(name_inactive_gas,0.5*(substrate_thickness+2*converter_thickness),0.5*substrate_width,0.5*inactive_gas_length0);
  place(shape_inact0,matGas,-(a-converter_thickness-0.5*substrate_thickness),0,0,lvGas,col_gasinact,0);

  //Then a curved part for outside (unfortunately we must use a boolean volume here):
  const double inactive_gas_length1 = detbox_length-2*endplug_thickness-2*substrate_endplug_dist;
  double th_inactbox1 = detbox_inner_radius-a;
  auto shape_inactbox1 = new G4Box("InActiveGasBoxShape",0.5*th_inactbox1,0.5*substrate_width,0.5*inactive_gas_length1);
  double inactbox1_pos = 0.5*(detbox_inner_radius+a);
  auto shape_inactivegas_curvedpart = new G4IntersectionSolid(name_inactive_gas,shape_gas,shape_inactbox1,0,G4ThreeVector(-inactbox1_pos,0,0));
  place(shape_inactivegas_curvedpart,matGas,0,0,0,lvGas,col_gasinact,1);

  return pvWorld;
}
