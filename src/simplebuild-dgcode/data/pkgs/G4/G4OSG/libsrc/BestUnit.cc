#include "G4OSG/BestUnit.hh"//for G4BestUnit (todo: dont depend on G4 here)
#include "G4UnitsTable.hh"//for G4BestUnit (todo: dont depend on G4 here)

std::string G4OSG::BestUnit(const double& value, const char * category)
{
  std::string s = G4BestUnit(value,category);
  while (s.size()&&s[s.size()-1]==' ')
    s.resize(s.size()-1);
  //Todo: Ang -> Aa or real angstrom character?
  return s;
}
