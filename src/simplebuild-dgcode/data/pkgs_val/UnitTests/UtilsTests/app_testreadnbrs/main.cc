#include <sstream>
#include <iostream>
#include <stdexcept>

#include "Utils/ReadAsciiNumbers.hh"

void write_test(const char* content,size_t nexcept) {

  std::ofstream f;
  f.open("write_test.txt");
  f << content;
  f.close();
  std::cout << "Wrote content >>"<<content<<"<<"<<std::endl;
  std::vector<double> v;
  if (!Utils::read_numbers("write_test.txt",v))
    throw std::runtime_error("read error");
  std::cout << "Read "<<v.size()<<" numbers:";
  for (auto it=v.begin();it!=v.end();++it)
    std::cout << " " << *it;
  std::cout << std::endl << std::endl;
  if (nexcept!=v.size())
    throw std::runtime_error("Test failed!");

}


int main(int, char**) {
  write_test("\n\t 1.0 \n 1.23 -2e-4",3);
  write_test("\t1.0 \n 1.23 -2e-4\n",3);
  write_test("1.0 \n 1.23 -2e-4 \n",3);
  write_test(" 32.08568807173650 60.12510288065853 28.50478947368421",3);
  write_test("32.08568807173650 60.12510288065853 28.50478947368421\n",3);
  write_test(" 0.18\t-4.8\t100\n0.2\t-4.8\t0",6);
  write_test("0.18\t-4.8\t100\n0.2\t-4.8\t0\n",6);
  write_test(" 0.18\t-4.8\t100\n0.2\t-4.8\t0 ",6);
  return 0;
}
