#include "G4B10Common/GeoB10Base.hh"

GeoB10Base::GeoB10Base(const char* name)
  : GeoConstructBase(name)
{
  addParameterString("material_countgas", "IdealGas:formula=0.7*Ar+0.3*CO2");
  addParameterString("material_substrate", "stdlib::Al_sg225.ncmat");
  addParameterString("material_detectorbox", "stdlib::Al_sg225.ncmat");
  addParameterString("material_converter","MAT_B4C:b10_enrichment=0.98");
  addParameterString("material_world","gasmix::air");//Another common option would be "Vacuum"
  addParameterDouble("vis_transparency", 0.0, 0.0, 1.0);//transparency of all colours
}

GeoB10Base::~GeoB10Base()
{
}

bool GeoB10Base::validateParameters()
{
  return true;
}

G4Material * GeoB10Base::createMaterialCountingGas() const
{
  return getParameterMaterial("material_countgas");
}

G4Material * GeoB10Base::createMaterialConverter() const
{
  return getParameterMaterial("material_converter");
}

G4Material * GeoB10Base::createMaterialSubstrate() const
{
  return getParameterMaterial("material_substrate");
}

G4Material * GeoB10Base::createMaterialDetectorBox() const
{
  return getParameterMaterial("material_detectorbox");
}

G4Material * GeoB10Base::createMaterialWorld() const
{
  return getParameterMaterial("material_world");
}

G4Colour GeoB10Base::colour_World() const
{
  return INVISIBLE;
}

G4Colour GeoB10Base::colour_Converter() const
{
  return G4Colour(1.0, 0.0,0.0,colour_alpha());
}

G4Colour GeoB10Base::colour_CountingGas() const
{
  return G4Colour(0.0, 0.0, 1.0,colour_alpha());
}

G4Colour GeoB10Base::colour_InactiveCountingGas() const
{
  return colour_CountingGas();//hardwired similar for now...
}

G4Colour GeoB10Base::colour_Substrate() const
{
  return G4Colour(.75, .55, 0.0,colour_alpha());
}

G4Colour GeoB10Base::colour_DetectorBox() const
{
  return G4Colour(0.0, 1.0, 0.0,colour_alpha());
}

double GeoB10Base::colour_alpha() const
{
  return 1.0-getParameterDouble("vis_transparency");
}
