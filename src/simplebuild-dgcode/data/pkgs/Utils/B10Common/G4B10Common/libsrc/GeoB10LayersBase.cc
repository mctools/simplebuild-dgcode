#include "G4B10Common/GeoB10LayersBase.hh"

GeoB10LayersBase::GeoB10LayersBase(const char* name)
  : GeoB10Base(name)
{
  addParameterDouble("detector_shift_z_mm", 1.0, -10000.0, 10000.0);
  addParameterDouble("detbox_wallthickness_mm", 5, 0.01, 50.0);
  addParameterDouble("converter_thickness_um", 1.0, 0.001, 50.0);
  addParameterDouble("substrate_thickness_mm", 0.5, 0.01, 10.0);
  addParameterDouble("gas_thickness_mm", 3, 0.01, 100.0);
  addParameterInt("number_layers", 30,1,500);
}

GeoB10LayersBase::~GeoB10LayersBase()
{
}

bool GeoB10LayersBase::validateParameters()
{
  if (!GeoB10Base::validateParameters())
    return false;
  //do we need more validation here?
  return true;
}

G4int GeoB10LayersBase::copyNumber(G4int layernumber,G4int cellnumber)
{
  assert(cellnumber>=0&&cellnumber<INT_MAX/10000);
  assert(layernumber>=0&&layernumber<10000);
  return layernumber + 10000*cellnumber;
}
