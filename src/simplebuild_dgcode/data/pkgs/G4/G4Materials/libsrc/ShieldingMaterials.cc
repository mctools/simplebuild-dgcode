#include "G4Materials/ShieldingMaterials.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include "G4Materials/CommonMaterials.hh"

void ShieldingMaterials::getMatNames(std::vector<std::string>&v)
{
  v.push_back("SHIELDING_paraffin_wax");
  v.push_back("SHIELDING_mild_steel");
  v.push_back("SHIELDING_He3_gas");
  v.push_back("SHIELDING_ordinary_concrete_psi");
  v.push_back("SHIELDING_borkron");
  v.push_back("SHIELDING_EXPERIMENTAL_brass");
  v.push_back("SHIELDING_heavy_water");
  v.push_back("SHIELDING_AlMg3");
  v.push_back("SHIELDING_zircaloy");
  v.push_back("SHIELDING_steel_2");
  v.push_back("SHIELDING_Be_TS");
  v.push_back("SHIELDING_Al_TS");
  v.push_back("SHIELDING_STEEL_TDR_TS");
  v.push_back("SHIELDING_Fe_TS");
  v.push_back("SHIELDING_LiqH2_TS");
  v.push_back("SHIELDING_H2O_TS");
  v.push_back("SHIELDING_AL6061_TDR_TS");
  v.push_back("SHIELDING_wood");
  v.push_back("SHIELDING_He3_wendi");
  v.push_back("SHIELDING_tungsten_powder");
  v.push_back("SHIELDING_TSL_AL6082_T6");
  v.push_back("SHIELDING_TSL_Cu_HCP");
  v.push_back("SHIELDING_TSL_Fe_CK45");
  v.push_back("SHIELDING_Heavy_Concrete");

}

G4Material* ShieldingMaterials::createMat(const std::string& name)
{
  // elements 
  G4Element*H = CommonMaterials::getNISTElement("H");
  G4Element*C = CommonMaterials::getNISTElement("C");
  G4Element*B = CommonMaterials::getNISTElement("B");
  G4Element*P = CommonMaterials::getNISTElement("P");
  G4Element*S = CommonMaterials::getNISTElement("S");
  G4Element*Si = CommonMaterials::getNISTElement("Si");
  G4Element*Cu = CommonMaterials::getNISTElement("Cu");
  G4Element*Fe = CommonMaterials::getNISTElement("Fe");
  G4Element*He3 = new G4Element("He3", "He3", 1);
  G4Element*Al = CommonMaterials::getNISTElement("Al");
  G4Element*Ca = CommonMaterials::getNISTElement("Ca");
  G4Element* O16 = new G4Element("O16", "O16", 1);
  G4Element*O = CommonMaterials::getNISTElement("O");
  G4Element*Na = CommonMaterials::getNISTElement("Na");
  G4Element*K = CommonMaterials::getNISTElement("K");
  G4Element*Zn = CommonMaterials::getNISTElement("Zn");
  G4Element* H2 = new G4Element("H2", "H2", 1);
  G4Element*V = CommonMaterials::getNISTElement("V");
  G4Element*Cr = CommonMaterials::getNISTElement("Cr");
  G4Element*Mn = CommonMaterials::getNISTElement("Mn");
  G4Element*Mg = CommonMaterials::getNISTElement("Mg");
  G4Element*Ti = CommonMaterials::getNISTElement("Ti");
  G4Element* O17 = new G4Element("O17", "O17", 1);
  G4Element* N14 = new G4Element("N14","N14", 1);
  G4Element* N15 = new G4Element("N15","N15", 1);
  G4Element*Hf = CommonMaterials::getNISTElement("Hf");
  G4Element*Sn = CommonMaterials::getNISTElement("Sn");
  G4Element*Zr = CommonMaterials::getNISTElement("Zr");
  G4Element*Ni = CommonMaterials::getNISTElement("Ni");
  G4Element*Mo = CommonMaterials::getNISTElement("Mo");
  G4Element*Co = CommonMaterials::getNISTElement("Co");
  G4Element*W = CommonMaterials::getNISTElement("W");
  G4Element*Ag = CommonMaterials::getNISTElement("Ag");
  G4Element*Bi = CommonMaterials::getNISTElement("Bi");
  G4Element*Pb = CommonMaterials::getNISTElement("Pb");


  // TS elements
  //FIXME: Leaking any of these not used!
  G4Element* elTSBe = 
    new G4Element( "TS_Beryllium_Metal" , "Be_TS" , 4 , 9.012182*g/mole );
  G4Element* elTSAl = 
    new G4Element( "TS_Aluminium_Metal" , "Al_TS" , 13 , 26.9815386*g/mole );
  G4Element* elTSFe = new G4Element("TS_Iron_Metal", "Fe_TS", 26, 55.845*g/mole);
  G4Element *elTSH = new G4Element("TS_H_of_Para_Hydrogen", "H_TS", 1., 1.00794*g/mole);
  G4Element *elTSH_water = new G4Element("TS_H_of_Water", "H_water_TS", 1., 1.00794*g/mole);

  // isotopes
  G4Isotope *He_3 = new G4Isotope("He_3", 2, 3, 3.02*g/mole);
  He3 -> AddIsotope(He_3, 100.*perCent);

  G4Isotope *O_16 = new G4Isotope("O_16", 8, 16, 15.9949*g/mole);
  O16 -> AddIsotope(O_16, 100.*perCent);

  G4Isotope *H_2 = new G4Isotope("H_2", 1, 2, 2.0141018*g/mole);
  H2 -> AddIsotope(H_2, 100.*perCent);

  G4Isotope *O_17 = new G4Isotope("O_17", 8, 17, 16.999131*g/mole);
  O17->AddIsotope(O_17, 100.*perCent);

  G4Isotope *N_14 = new G4Isotope("N_14", 7, 14, 14.0030740*g/mole);
  N14->AddIsotope(N_14, 100.*perCent);

  G4Isotope *N_15 = new G4Isotope("N_15", 7, 15, 15.0001089*g/mole);
  N15->AddIsotope(N_15, 100.*perCent);


  // materials
  G4Material * mat(0);

  if (name=="SHIELDING_paraffin_wax") {
    mat = new G4Material(name, 0.931*g/cm3, 3);
    mat -> AddElement(H, (14.8605-1.5)*perCent);
    mat -> AddElement(C, (85.1395-1.5)*perCent);
    mat -> AddElement(B, 3.0*perCent);

  } else if (name=="SHIELDING_mild_steel") {
    mat = new G4Material(name, 7.85*g/cm3, 6);
    mat -> AddElement(C, 0.26*perCent);
    mat -> AddElement(P, 0.04*perCent);
    mat -> AddElement(S, 0.05*perCent);
    mat -> AddElement(Si, 0.4*perCent);
    mat -> AddElement(Cu, 0.2*perCent);
    mat -> AddElement(Fe, 99.05*perCent);

  } else if (name=="SHIELDING_He3_gas") {
    mat = new G4Material(name, 0.0006193*g/cm3, 1, kStateGas, 293*kelvin,  4.93*atmosphere);
    mat -> AddElement(He3, 100*perCent);

  } else if (name=="SHIELDING_ordinary_concrete_psi") {
    mat = new G4Material(name, 2.5*g/cm3, 5);
    mat -> AddElement (H, 0.33*perCent);
    mat -> AddElement (Si, 19.42*perCent);
    mat -> AddElement (Ca, 19.42*perCent);
    mat -> AddElement (Al, 4.86*perCent);
    mat -> AddElement (O16, 55.97*perCent);

  } else if (name=="SHIELDING_borkron") {
    mat = new G4Material(name, 2.39*g/cm3, 5);
    mat -> AddElement(Si, 30*perCent);
    mat -> AddElement(O, 45*perCent);
    mat -> AddElement(B, 15*perCent);
    mat -> AddElement(Na, 5*perCent);
    mat -> AddElement(K, 5*perCent);

  } else if (name=="SHIELDING_EXPERIMENTAL_brass") {
    mat = new G4Material(name, 8.5*g/cm3, 2);
    mat -> AddElement(Cu, 63*perCent);
    mat -> AddElement(Zn, 37*perCent);

  } else if (name=="SHIELDING_heavy_water") {
    mat = new G4Material(name, 2.3*g/cm3, 2);
    mat -> AddElement (H2, 2);
    mat -> AddElement (O, 1);

  } else if (name=="SHIELDING_AlMg3") {
    mat = new G4Material(name, 2.66*g/cm3, 10);
    mat -> AddElement (Al, 96.36*perCent);
    mat -> AddElement (Zn, 0.054*perCent);
    mat -> AddElement (Fe, 0.232*perCent);
    mat -> AddElement (V,  0.00005*perCent);
    mat -> AddElement (Si, 0.056*perCent);
    mat -> AddElement (Cr, 0.037*perCent);
    mat -> AddElement (Cu, 0.03*perCent);
    mat -> AddElement (Mn, 0.313*perCent);
    mat -> AddElement (Mg, 2.9*perCent);
    mat -> AddElement (Ti, 0.013*perCent);

  } else if (name=="SHIELDING_zircaloy") {
    mat = new G4Material(name, 6.56*g/cm3, 14);
    mat->AddElement (Zr, 97.89934*perCent);
    mat->AddElement (Cr, 0.108*perCent);
    mat->AddElement (Sn, 1.516*perCent);
    mat->AddElement (O17, 0.00004894*perCent);
    mat->AddElement (Hf, 0.0105*perCent);
    mat->AddElement (Si, 0.00237*perCent);
    mat->AddElement (Fe, 0.176*perCent);
    mat->AddElement (Al, 0.0007037*perCent);
    mat->AddElement (C, 0.001539*perCent);
    mat->AddElement (N14, 0.000433*perCent);
    mat->AddElement (Ni, 0.174*perCent);
    mat->AddElement (O16, 0.12235*perCent);
    mat->AddElement (H, 0.00000328*perCent);
    mat->AddElement (N15, 0.00000159*perCent);

  } else if (name=="SHIELDING_steel_2") {
    mat = new G4Material(name, 7.96*g/cm3, 16);
    mat->AddElement (P, 0.022*perCent);
    mat->AddElement (S, 0.002*perCent);
    mat->AddElement (Sn, 0.009*perCent);
    mat->AddElement (Zn, 0.005*perCent);
    mat->AddElement (Co, 0.178*perCent);
    mat->AddElement (Ni, 12*perCent);
    mat->AddElement (Si, 0.477*perCent);
    mat->AddElement (Mn, 1.71*perCent);
    mat->AddElement (Cr, 16.8*perCent);
    mat->AddElement (Mo, 2.55*perCent);
    mat->AddElement (V, 0.0522*perCent);
    mat->AddElement (Cu, 0.361*perCent);
    mat->AddElement (Ti, 0.0346*perCent);
    mat->AddElement (Fe, 65.7718*perCent);
    mat->AddElement (Al, 0.0044*perCent);
    mat->AddElement (C, 0.023*perCent);

  } else if (name=="SHIELDING_Be_TS") {
    mat = new G4Material(name,1.85*g/cm3 , 1);
    mat->AddElement(elTSBe,1);

  } else if (name=="SHIELDING_Al_TS") {
    mat = new G4Material(name,2.700*g/cm3 ,  1);
    mat->AddElement(elTSAl,1);

  } else if (name=="SHIELDING_STEEL_TDR_TS") {
    mat = new G4Material(name,7.85*g/cm3,10 );
    mat->AddElement (C, 0.03*perCent);
    mat->AddElement (Cr, 17.0*perCent);
    mat->AddElement (elTSFe, 65.7*perCent);
    mat->AddElement (Mn, 1.80*perCent);
    mat->AddElement (Mo, 2.5*perCent);
    mat->AddElement (Ni, 12.5*perCent);
    mat->AddElement (Si, 0.4*perCent);
    mat->AddElement (Co, 0.03*perCent);
    mat->AddElement (P, 0.02*perCent);
    mat->AddElement (S, 0.01*perCent);

  }  else if (name=="SHIELDING_Fe_TS") {
    mat = new G4Material(name, 7.85*g/cm3 , 1);
    mat->AddElement(elTSFe,1);

  } else if (name=="SHIELDING_LiqH2_TS") {
    mat = new G4Material(name,70.0*mg/cm3, 1, kStateLiquid, 20.0*kelvin,1500000.0*pascal);
    mat->AddElement(elTSH,1);

  } else if (name=="SHIELDING_H2O_TS") {
    mat = new G4Material(name,1.000*g/cm3,2,kStateLiquid,293.0*kelvin);
    mat->AddElement(elTSH_water, 2);
    mat->AddElement(O, 1);

  } else if (name=="SHIELDING_AL6061_TDR_TS") {
    mat = new G4Material(name,2.7*g/cm3,9);
    mat->AddElement (elTSAl, 97.3*perCent);
    mat->AddElement (Cr, 0.195*perCent);
    mat->AddElement (Cu, 0.275*perCent);
    mat->AddElement (Fe, 0.35*perCent);
    mat->AddElement (Mg, 1.0*perCent);
    mat->AddElement (Mn, 0.075*perCent);
    mat->AddElement (Si, 0.6*perCent);
    mat->AddElement (Ti, 0.075*perCent);
    mat->AddElement (Zn, 0.125*perCent);

  } else if (name=="SHIELDING_wood") {
    mat = new G4Material(name, 0.63*g/cm3, 3);
    mat->AddElement (H, 6*perCent);
    mat->AddElement (C, 54*perCent);
    mat->AddElement (O, 40*perCent);

  } else if (name=="SHIELDING_He3_wendi") {
    mat = new G4Material(name, 0.0002513*g/cm3, 1, kStateGas, 293*kelvin,  2*atmosphere);
    mat -> AddElement(He3, 100*perCent);

  } else if (name=="SHIELDING_tungsten_powder") {
    mat = new G4Material(name, 10.624*g/cm3, 1);
    mat -> AddElement(W, 100*perCent);

  } else if (name=="SHIELDING_TSL_AL6082_T6") {
    mat = new G4Material(name, 2.7*g/cm3, 9);
    mat->AddElement (Al, 97.15*perCent);
    mat->AddElement (Cr, 0.25*perCent);
    mat->AddElement (Cu, 0.1*perCent);
    mat->AddElement (Fe, 0.5*perCent);
    mat->AddElement (Mg, 0.6*perCent);
    mat->AddElement (Mn, 0.4*perCent);
    mat->AddElement (Si, 0.7*perCent);
    mat->AddElement (Ti, 0.1*perCent);
    mat->AddElement (Zn, 0.2*perCent);
  
  } else if (name=="SHIELDING_TSL_Cu_HCP") {
    mat = new G4Material(name, 8.94*g/cm3, 6);
    mat->AddElement (Cu, 99.95*perCent);
    mat->AddElement (Ag, 0.015*perCent);
    mat->AddElement (P, 0.002*perCent);
    mat->AddElement (Bi, 0.0005*perCent);
    mat->AddElement (Pb, 0.005*perCent);
    mat->AddElement (O, 0.0275*perCent);
 
 } else if (name=="SHIELDING_TSL_Fe_CK45") {
    mat = new G4Material(name, 7.84*g/cm3, 7);
    mat->AddElement (Fe, 97.86*perCent);
    mat->AddElement (C, 0.46*perCent);
    mat->AddElement (Si, 0.4*perCent);
    mat->AddElement (Mn, 0.65*perCent);
    mat->AddElement (Cr, 0.28*perCent);
    mat->AddElement (Mo, 0.07*perCent);
    mat->AddElement (Ni, 0.28*perCent);

  } else if (name=="SHIELDING_Heavy_Concrete") {
    mat = new G4Material(name, 3.68*g/cm3, 10);
    mat->AddElement (H, 0.53*perCent);    
    mat->AddElement (O, 33.2*perCent);
    mat->AddElement (Na, 0.46*perCent);
    mat->AddElement (Al, 0.64*perCent);
    mat->AddElement (C, 4.69*perCent);
    mat->AddElement (P, 0.44*perCent);
    mat->AddElement (S, 0.03*perCent);
    mat->AddElement (K, 0.15*perCent);
    mat->AddElement (Ca, 1.98*perCent);
    mat->AddElement (Fe, 57.88*perCent);
  }
    
  return mat;
}
