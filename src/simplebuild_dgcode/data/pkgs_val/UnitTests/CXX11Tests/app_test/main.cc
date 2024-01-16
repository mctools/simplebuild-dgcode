#include <array>
#include <random>
#include <iostream>

int main(int,char**) {

  // Just a silly little bit of C++11 code using features which only works on
  // newer compilers.

  std::array<double,7> a;
  a = { { 1, 2, 3, 4, 5, 6, 7 } };
  for (auto& e: a)
    std::cout << e << std::endl;

  std::default_random_engine gen;
  std::normal_distribution<double> dist1(0, 1.0);
  std::lognormal_distribution<double> dist2(0, 2.0);

  std::cout << dist1(gen) << std::endl;
  std::cout << dist2(gen) << std::endl;

  int i1(1);
  unsigned i2(2);
  long int i3(3);
  long unsigned i4(4);
  long long int i5(5);
  long long unsigned i6(6);

  std::cout << std::to_string(i1) << std::endl;
  std::cout << std::to_string(i2) << std::endl;
  std::cout << std::to_string(i3) << std::endl;
  std::cout << std::to_string(i4) << std::endl;
  std::cout << std::to_string(i5) << std::endl;
  std::cout << std::to_string(i6) << std::endl;

  return 0;
}

