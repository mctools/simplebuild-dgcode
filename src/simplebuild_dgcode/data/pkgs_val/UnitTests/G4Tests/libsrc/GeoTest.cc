#include "G4Tests/GeoTest.hh"
#include "Units/Units.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Transform3D.hh"
#include "G4VisAttributes.hh"

G4VPhysicalVolume* G4Tests::GeoTest::Construct()
{
  //free parameters:
  double boron_thickness = getParameterDouble("boronThickness_micron")*Units::um;
  double world_extent = getParameterDouble("worldExtent_meters")*Units::meter;

  //materials
  auto mat_vacuum = getMaterial("Vacuum");
  auto mat_boron = getMaterial("G4_B");

  //world
  auto sld_world = new G4Box("world", world_extent,world_extent,world_extent*2);
  auto lv_world = new G4LogicalVolume(sld_world, mat_vacuum, "world");
  auto pv_world = new G4PVPlacement(G4Transform3D(),lv_world, "world",0, false, 0);

  //target box
  auto sld_targetbox = new G4Box("sld_targetbox", world_extent,world_extent,boron_thickness*0.5);
  auto lv_targetbox = new G4LogicalVolume(sld_targetbox, mat_boron, "lv_targetbox",0);
  new G4PVPlacement(0, G4ThreeVector(0.0,0.0,0.0),"pv_targetbox", lv_targetbox, pv_world, false, 0, true);
  lv_targetbox->SetVisAttributes(new G4VisAttributes(RED));

  //empty recording box lv:
  auto sld_recordingbox = new G4Box("sld_recordingbox", world_extent,world_extent,0.5*(world_extent-boron_thickness*0.5));
  auto lv_recordingbox = new G4LogicalVolume(sld_recordingbox, mat_vacuum, "lv_recordingbox",0);
  lv_recordingbox->SetVisAttributes(new G4VisAttributes(BLUE));

  //place empty recording boxes before and after the target box:
  new G4PVPlacement(0, G4ThreeVector(0.0,0.0,-0.5*world_extent-boron_thickness*0.25),
                    "pv_recordingbox_before", lv_recordingbox, pv_world, false, 0, true);
  new G4PVPlacement(0, G4ThreeVector(0.0,0.0,+0.5*world_extent+boron_thickness*0.25),
                    "pv_recordingbox_after", lv_recordingbox, pv_world, false, 1, true);

  return pv_world;
}
