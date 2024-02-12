//<SKEL_MUST_MODIFY_FILE>

/////////////////////////////////////////
// Declaration of our geometry module: //
/////////////////////////////////////////

#include "G4Interfaces/GeoConstructPyExport.hh"

namespace {
  class GeoSkeletonSP final : public G4Interfaces::GeoConstructBase {
  public:
    GeoSkeletonSP();
    G4VPhysicalVolume* Construct() override;
    //(add more member functions and data here if needed)
  };
}

PYTHON_MODULE( mod )
{
  GeoConstructPyExport::exportGeo<GeoSkeletonSP>(mod, "GeoSkeletonSP");
}

////////////////////////////////////////////
// Implementation of our geometry module: //
////////////////////////////////////////////

#include "G4Box.hh"
#include "G4Orb.hh"

//As an example, we put a spherical sample in front of a box representing a
//detector tube (using the Griff analysis to look for positions where neutrons
//enter the detector).

GeoSkeletonSP::GeoSkeletonSP()
  : GeoConstructBase("G4GeoSkeletonSP/GeoSkeletonSP")
{
  //Free parameters, tunable from python and command-line, printed in log-files,
  //stored in Griff-files. The values specified below will be the default
  //values. Avoid the temptation to instead hard-code "interesting" parameters
  //in the Construct() method:

  addParameterDouble("sample_posz_mm",5.0);
  addParameterDouble("sample_radius_mm",5.0);
  addParameterDouble("detector_size_cm",50.0);
  addParameterDouble("detector_sample_dist_cm",10.0);
  addParameterString("material_sample","stdlib::Al_sg225.ncmat");
  addParameterString("material_lab",
                     "gasmix::0.7xAr+0.3xCO2/2.0atm/293.15K");

  //Note: It is possible and easy (but not done here to keep the example simple)
  //      to impose constraints on parameter ranges, etc., making sure the
  //      geometry will only be build with sensible parameters. For instance,
  //      one should not put the sample radius larger than the detector-sample
  //      distance.
}

G4VPhysicalVolume* GeoSkeletonSP::Construct()
{
  //Parameters (converting to G4 units immediately as is best practice):
  const double sample_posz = getParameterDouble("sample_posz_mm")*Units::mm;
  const double sample_radius = getParameterDouble("sample_radius_mm")*Units::mm;
  const double det_size = getParameterDouble("detector_size_cm")*Units::cm;
  const double det_depth = 1.0*Units::cm;//ok to hardcode non-interesting parameters
  const double det_sample_dist = getParameterDouble("detector_sample_dist_cm")*Units::cm;

  //Materials are set up in the following via simple strings (in the case of
  //mat_det, it is hardcoded to always be a vacuum). This has many advantages.
  //Find more info at: https://mctools.github.io/simplebuild-dgcode/matdef.html

  auto mat_sample = getParameterMaterial("material_sample");
  auto mat_lab = getParameterMaterial("material_lab");
  auto mat_det = getMaterial("Vacuum");

  //World volume (must be big enough for the sample and detector to fit inside):
  const double dz_world = 1.001 * ( std::abs<double>(sample_posz)
                                    + sample_radius+det_depth+det_sample_dist );
  const double dxy_world = 1.001 * std::max<double>(0.5*det_size,sample_radius);
  auto worldvols = place( new G4Box("World", dxy_world, dxy_world, dz_world),
                          mat_lab );
  auto lvWorld = worldvols.logvol;

  //Sample:
  place( new G4Orb("Sample",sample_radius),
         mat_sample,0,0,sample_posz,lvWorld,YELLOW );

  //Detector:
  place(new G4Box("Detector",0.5*det_size,0.5*det_size,0.5*det_depth),mat_det,
        0,0,sample_posz+det_sample_dist+0.5*det_depth,lvWorld,RED);

  //Note that we added visualisation colours for volumes above. Other colours
  //include BLUE, GREEN, ORANGE, CYAN, PURPLE, SILVER, GOLD, WHITE, BROWN, ...

  return worldvols.physvol;
}
