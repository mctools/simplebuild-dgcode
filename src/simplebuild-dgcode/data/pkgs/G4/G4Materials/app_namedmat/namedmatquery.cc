#include "G4Materials/NamedMaterialProvider.hh"
#include "G4Material.hh"
#include <iostream>

int main(int argc,char**argv) {
  if (argc<2) {
    std::cout<<"Please specify one or more named material definition strings on the command line"<<std::endl;
    return 1;
  }
  for (int i = 1; i<argc;++i) {
    G4Material * mat = NamedMaterialProvider::getMaterial(argv[i]);
    std::cout<<"==================================================================================="<<std::endl;
    std::cout<<"Material defined by string \""<<argv[i]<<"\":"<<std::endl;
    std::cout<<std::endl;
    std::cout<<*mat<<std::endl;
    std::cout<<"==================================================================================="<<std::endl;
  }
  return 0;
}
