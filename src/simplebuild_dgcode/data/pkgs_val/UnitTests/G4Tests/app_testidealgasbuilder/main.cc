#include "IdealGasBuilder/IdealGasBuilder.hh"
#include "G4NistManager.hh"
#include "G4PhysicalConstants.hh"
#include <iostream>

int main(int,char**) {

  //Some natural elements:
  auto el_O = G4NistManager::Instance()->FindOrBuildElement("O",true);
  auto el_C = G4NistManager::Instance()->FindOrBuildElement("C",true);
  auto el_F = G4NistManager::Instance()->FindOrBuildElement("F",true);
  auto el_Ar = G4NistManager::Instance()->FindOrBuildElement("Ar",true);
  auto el_He = G4NistManager::Instance()->FindOrBuildElement("He",true);

  //Enriched helium (90% He3):
  auto iso_he3 = new G4Isotope("He3", 2, 3, 3.016029*CLHEP::g/CLHEP::mole);
  auto iso_he4 = new G4Isotope("He4", 2, 4, 4.002602*CLHEP::g/CLHEP::mole);
  auto el_enriched_helium = new G4Element("EnrichedHe","He",2);
  el_enriched_helium->AddIsotope(iso_he3,0.9);
  el_enriched_helium->AddIsotope(iso_he4,0.1);

  //Print molar masses of elements:
  printf("Molar mass of C: %g g\n",el_C->GetA()/CLHEP::Avogadro/CLHEP::amu);
  printf("Molar mass of O: %g g\n",el_O->GetA()/CLHEP::Avogadro/CLHEP::amu);
  printf("Molar mass of F: %g g\n",el_F->GetA()/CLHEP::Avogadro/CLHEP::amu);
  printf("Molar mass of Ar: %g g\n",el_Ar->GetA()/CLHEP::Avogadro/CLHEP::amu);
  printf("Molar mass of He: %g g\n",el_He->GetA()/CLHEP::Avogadro/CLHEP::amu);
  printf("Molar mass of enriched He: %g g\n",el_enriched_helium->GetA()/CLHEP::Avogadro/CLHEP::amu);

  //Use elements to create some often used components:
  IdealGas::Component co2("CO2");
  IdealGas::Component o2;
  o2.addElement("O",2.0);
  IdealGas::Component cf4;
  cf4.addElement(el_C,1.0);
  cf4.addElement("F",4.0);
  cf4.lock();
  IdealGas::Component ar;
  ar.addElement(el_Ar,1.0);
  IdealGas::Component he;
  he.addElement(el_He,1.0);
  IdealGas::Component enriched_he;
  enriched_he.addElement(el_enriched_helium,1.0);

  //More versions of CO2, to test the addElements code:
  IdealGas::Component co2_v2;
  co2_v2.addElement(el_C,1.0);
  co2_v2.addElement(el_O,2.0);
  IdealGas::Component co2_v3;
  co2_v3.addElements("O2C1.0");
  IdealGas::Component co2_v4;
  co2_v4.addElements("O2.0C");
  IdealGas::Component co2_v5;
  co2_v5.addElements("COO");
  IdealGas::Component co2_v6;
  co2_v6.addElement("O",0.4);
  co2_v6.addElements("O1.6C");
  IdealGas::Component co2_v7;
  co2_v7.addElements("C0.5O1.1");
  co2_v7.addElements("C0.5O0.9");

  //Print molecular molar masses of components:
  printf("Molar mass of component %s (v1): %g g\n",co2.name(),co2.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s (v2): %g g\n",co2_v2.name(),co2.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s (v3): %g g\n",co2_v3.name(),co2.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s (v4): %g g\n",co2_v4.name(),co2.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s (v5): %g g\n",co2_v5.name(),co2.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s (v6): %g g\n",co2_v6.name(),co2.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s (v7): %g g\n",co2_v7.name(),co2.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s: %g g\n",o2.name(),o2.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s: %g g\n",cf4.name(),cf4.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s: %g g\n",ar.name(),ar.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s: %g g\n",he.name(),he.molarMass()/CLHEP::gram);
  printf("Molar mass of component %s: %g g\n",enriched_he.name(),enriched_he.molarMass()/CLHEP::gram);

  //Create some example gas mixtures:
  IdealGas::Mixture gas1;
  gas1.addComponent(ar,0.7);
  gas1.addComponent(co2,0.2);
  gas1.addComponent(o2,0.1);

  G4Material * g4mat_gas1 = gas1.createMaterial(-1,-1,"Gas1");

  IdealGas::Mixture gas2;
  gas2.addComponent(ar,0.7);
  gas2.addComponent("CO2",0.27);
  gas2.addComponent(cf4,0.03);
  G4Material * g4mat_gas2 = gas2.createMaterial();

  IdealGas::Mixture gas3;
  gas3.addComponents("CF4");
  G4Material * g4mat_gas3 = gas3.createMaterial(-1,-1,"Gas3");

  IdealGas::Mixture gas4;
  gas4.addComponent(enriched_he,1.0);
  G4Material * g4mat_gas4 = gas4.createMaterial();

  IdealGas::Mixture gas5;
  gas5.addComponent(he,1.0);
  G4Material * g4mat_gas5 = gas5.createMaterial(-1,-1,"Gas5");

  //Dump the results:
  std::cout << g4mat_gas1 << std::endl;
  std::cout << g4mat_gas2 << std::endl;
  std::cout << g4mat_gas3 << std::endl;
  std::cout << g4mat_gas4 << std::endl;
  std::cout << g4mat_gas5 << std::endl;

  std::cout << IdealGas::createMaterial("0.7*Ar+0.2*CO2+0.1*O2") << std::endl;
  std::cout << IdealGas::createMaterial("0.2*CO2+0.1*O2+0.7*Ar") << std::endl;
  std::cout << IdealGas::createMaterial("0.7*Ar+0.2*CO2+0.1*O2",20*CLHEP::kelvin) << std::endl;
  std::cout << IdealGas::createMaterial("0.7*Ar+0.2*CO2+0.1*O2",-1,10*CLHEP::bar) << std::endl;
  std::cout << IdealGas::createMaterial("0.7*Ar+0.2*CO2+0.1*O2",20*CLHEP::kelvin,10*CLHEP::bar) << std::endl;
  std::cout << IdealGas::createMaterial("0.2*Ar+0.75*B{10|0.95}F3+0.04*CH4+0.01*N2",293.15*CLHEP::kelvin,2.0*CLHEP::bar) <<std::endl;
  std::cout << IdealGas::createMaterial("He{3|0.98}",-1,1.5*CLHEP::bar) << std::endl;
  std::cout << IdealGas::createMaterial("0.1*H{3|0.2,2|0.6}2O+0.9*O2",500*CLHEP::kelvin) << std::endl;//heated oxygen with a spray of deuterated and tritiated water

  std::cout << gas2.createMaterialCalcD(200*CLHEP::kelvin,2.0*CLHEP::atmosphere,"Gas2 / CalcD") << std::endl;
  double gas2_density = 5.17760144788*CLHEP::kg/CLHEP::m3;
  std::cout << gas2.createMaterialCalcP(gas2_density,200*CLHEP::kelvin,"Gas2 / CalcP") << std::endl;
  std::cout << gas2.createMaterialCalcT(gas2_density,2.0*CLHEP::atmosphere,"Gas2 / CalcT") << std::endl;
  return 0;
}
