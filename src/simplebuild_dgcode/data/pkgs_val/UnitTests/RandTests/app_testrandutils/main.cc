#include "RandUtils/Rand.hh"
#include <iostream>

int main(int,char**) {
  RandUtils::Rand r(117);
  for (unsigned i = 0; i < 20; ++i)
    std::cout << r.shoot() << std::endl;
  for (unsigned i = 0; i < 20; ++i)
    std::cout << r.shootUInt32() << std::endl;
  return 0;
}
