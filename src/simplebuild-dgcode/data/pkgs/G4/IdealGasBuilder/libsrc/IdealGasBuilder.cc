//See IdealGasBuilder.hh for general information.
//Author: Thomas Kittelmann, January 2015.

#include "IdealGasBuilder/IdealGasBuilder.hh"

G4Material * IdealGas::createMaterialCalcD(const char* formula,double temp, double pressure, const char * name)
{
  Mixture mix(formula);
  return mix.createMaterialCalcD(temp,pressure,name);
}

G4Material * IdealGas::createMaterialCalcP(const char* formula,double density,double temp, const char * name)
{
  Mixture mix(formula);
  return mix.createMaterialCalcP(density,temp,name);
}

G4Material * IdealGas::createMaterialCalcT(const char* formula,double density, double pressure, const char * name)
{
  Mixture mix(formula);
  return mix.createMaterialCalcT(density,pressure,name);
}

G4Material * IdealGas::createMaterial(const char* formula,double temp, double pressure, const char * name)
{
  Mixture mix(formula);
  return mix.createMaterial(temp, pressure,name);
}
