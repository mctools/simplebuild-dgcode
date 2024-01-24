#include "G4Materials/CommonMaterials.hh"
#include "Units/Units.hh"
#include "Core/FPE.hh"
#include "init_dummy_geant4.hh"
#include <iostream>

void print(const char* str, const G4Material* mat) {
  assert(mat);
  printf("=========== G4Material %s ==========\n",str);
  std::cout<<const_cast<G4Material*>(mat)<<std::endl;
  printf("   ---> Temperature: %f kelvin\n",mat->GetTemperature()/Units::kelvin);
  printf("   ---> Pressure: %f bar\n",mat->GetPressure()/Units::bar);
  printf("   ---> State-solid: %s\n",mat->GetState()==kStateSolid ? "yes" : "no");
  printf("\n");
}

void print(const char* str, const G4Element* elem) {
  assert(elem);
  printf("=========== G4Element %s ==========\n",str);
  std::cout<<const_cast<G4Element*>(elem)<<std::endl<<std::endl;
}

void print(const char* str, const G4Isotope* iso) {
  assert(iso);
  printf("=========== G4Isotope %s ==========\n",str);
  std::cout<<const_cast<G4Isotope*>(iso)<<std::endl<<std::endl;
}

int main(int,char**) {

  Core::catch_fpe();

  init_dummy_geant4();

  assert(CommonMaterials::getNISTMaterial("G4_B")==CommonMaterials::getNISTMaterial("G4_B"));

  print("getNISTMaterial(\"G4_H\")",CommonMaterials::getNISTMaterial("G4_H"));
  print("getNISTMaterial(\"G4_He\")",CommonMaterials::getNISTMaterial("G4_He"));
  print("getNISTMaterial(\"G4_Li\")",CommonMaterials::getNISTMaterial("G4_Li"));
  print("getNISTMaterial(\"G4_Be\")",CommonMaterials::getNISTMaterial("G4_Be"));
  print("getNISTMaterial(\"G4_B\")",CommonMaterials::getNISTMaterial("G4_B"));
  print("getNISTMaterial(\"G4_C\")",CommonMaterials::getNISTMaterial("G4_C"));
  print("getNISTMaterial(\"G4_O\")",CommonMaterials::getNISTMaterial("G4_O"));
  print("getNISTMaterial(\"G4_Al\")",CommonMaterials::getNISTMaterial("G4_Al"));
  print("getNISTMaterial(\"G4_Gd\")",CommonMaterials::getNISTMaterial("G4_Gd"));
  print("getNISTMaterial(\"G4_Fe\")",CommonMaterials::getNISTMaterial("G4_Fe"));

  print("getNISTElement(\"H\")",CommonMaterials::getNISTElement("H"));
  print("getNISTElement(\"He\")",CommonMaterials::getNISTElement("He"));
  print("getNISTElement(\"Li\")",CommonMaterials::getNISTElement("Li"));
  print("getNISTElement(\"Be\")",CommonMaterials::getNISTElement("Be"));
  print("getNISTElement(\"B\")",CommonMaterials::getNISTElement("B"));
  print("getNISTElement(\"C\")",CommonMaterials::getNISTElement("C"));
  print("getNISTElement(\"O\")",CommonMaterials::getNISTElement("O"));
  print("getNISTElement(\"Al\")",CommonMaterials::getNISTElement("Al"));
  print("getNISTElement(\"Gd\")",CommonMaterials::getNISTElement("Gd"));
  print("getNISTElement(\"Fe\")",CommonMaterials::getNISTElement("Fe"));

  print("getIsotope_B10()",CommonMaterials::getIsotope_B10());
  print("getIsotope_B11()",CommonMaterials::getIsotope_B11());

  printf("=== getNaturalB10IsotopeFraction() = %f ===\n",CommonMaterials::getNaturalB10IsotopeFraction());
  printf("=== getNaturalDensity_Boron() = %f g/cm3 ===\n",CommonMaterials::getNaturalDensity_Boron()/(Units::g/Units::cm3));
  printf("=== getNaturalDensity_BoronCarbide() = %f g/cm3 ===\n",CommonMaterials::getNaturalDensity_BoronCarbide()/(Units::g/Units::cm3));

  print("getElement_Boron(0.9123456781234567)",
        CommonMaterials::getElement_Boron(0.9123456781234567));
  print("getElement_Boron()",CommonMaterials::getElement_Boron());
  print("getElement_Boron(0.0)",CommonMaterials::getElement_Boron(0.0));
  print("getElement_Boron(0.17)",CommonMaterials::getElement_Boron(0.17));
  print("getElement_Boron(1.0)",CommonMaterials::getElement_Boron(1.0));
  print("getElement_Boron(0.9123456781234567)",
        CommonMaterials::getElement_Boron(0.9123456781234567));//same again, to test cache

  print("getMaterial_Boron(0.9123456781234567)",
        CommonMaterials::getMaterial_Boron(0.9123456781234567));
  print("getMaterial_Boron()",CommonMaterials::getMaterial_Boron());
  print("getMaterial_Boron(0.0)",CommonMaterials::getMaterial_Boron(0.0));
  print("getMaterial_Boron(0.17)",CommonMaterials::getMaterial_Boron(0.17));
  print("getMaterial_Boron(1.0)",CommonMaterials::getMaterial_Boron(1.0));
  print("getMaterial_Boron(0.9123456781234567)",
        CommonMaterials::getMaterial_Boron(0.9123456781234567));//same again, to test cache

  print("getMaterial_Boron(-1,2.0g/cm3)",CommonMaterials::getMaterial_Boron(-1,2.0*Units::g/Units::cm3));
  print("getMaterial_Boron(0.17,2.0g/cm3)",CommonMaterials::getMaterial_Boron(0.17,2.0*Units::g/Units::cm3));

  print("getElement_Carbon()",CommonMaterials::getElement_Carbon());

  print("getMaterial_BoronCarbide()",CommonMaterials::getMaterial_BoronCarbide());
  print("getMaterial_BoronCarbide(0.9,-1,20kelvin)",CommonMaterials::getMaterial_BoronCarbide(0.9,-1,20*Units::kelvin));
  print("getMaterial_BoronCarbide(fb10nat*1.00000001)",
        CommonMaterials::getMaterial_BoronCarbide(CommonMaterials::getNaturalB10IsotopeFraction()*1.00000001));

  print("getMaterial_BoronCarbide(fb10nat*1.00000001)",
        CommonMaterials::getMaterial_BoronCarbide(CommonMaterials::getNaturalB10IsotopeFraction()*1.00000001));

  print("getMaterial_Vacuum()",CommonMaterials::getMaterial_Vacuum());

  double dn = CommonMaterials::getDensity_BoronCarbide(-1.0);
  printf("=========== CommonMaterials::getDensity_BoronCarbide ==========\n");
  for (int i=-1;i<=10;++i) {
    double fb10 = i/10.0;
    if (fb10<0) fb10=-1;
    double d = CommonMaterials::getDensity_BoronCarbide(fb10);
    printf("   fb10 = %g ---> %g g/mole (rel. diff %g %%)\n",
           fb10,
           d / (Units::g/Units::mole),
           100.0*(d - dn)/dn);
  }
}
