#include "GriffFormat/ParticleDefinition.hh"
#include "EvtFile/FileWriter.hh"

int main(int,char**)
{
  GriffFormat::ParticleDefinition a;
  a.mass = 1234.5;
  a.width = 5678.9;
  a.charge = 1716.1819e-199;
  a.lifeTime = 2021.2223e199;
  a.nameIdx = 0xFFFF;
  a.typeIdx = 17;
  a.subTypeIdx = 0x0a0a;
  a.pdgcode = 12;
  a.atomicNumber = -12;
  a.atomicMass = 2000000000;
  a.magneticMoment = 0.1e-17;
  a.spin_halfs=40;
  a.stable = 1;
  a.shortLived = 0;
  char lala[1024];
  a.writeRaw(&lala[0]);
  const char * data = &lala[0];
  GriffFormat::ParticleDefinition b(data);
  if (a!=b) {
    printf("GriffFormat::ParticleDefinition read/write test failed!\n");
    return 1;
  }
  printf("GriffFormat::ParticleDefinition read/write test ok\n");
  return 0;
}
