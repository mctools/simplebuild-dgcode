#include "Utils/Url2Local.hh"
#include <iostream>
int main( int argc, char** argv )
{
  if (argc<2||argc>3)
    return 1;
  std::string fn = argv[1];
  std::string cachedir = argc==3 ? argv[2] : "";
  std::cout<< Utils::url2local(fn, cachedir)<<std::endl;
  return 0;
}
