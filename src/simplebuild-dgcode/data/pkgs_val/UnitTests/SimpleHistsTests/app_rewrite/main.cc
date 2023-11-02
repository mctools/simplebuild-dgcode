#include "SimpleHists/HistCollection.hh"
#include <vector>
#include <algorithm>

int main(int argc,char** argv) {
  std::vector<std::string> args(argv+1, argv+argc);

  if ( args.size()!=1||std::find(args.begin(), args.end(), "-h") != args.end()
       || std::find(args.begin(), args.end(), "--help") != args.end() ) {
    printf("Usage: Run on .shist file to read and rewrite (thus converting to latest file version)\n");
    return 1;
  }

  SimpleHists::HistCollection hc(args.front().c_str());
  hc.saveToFile(args.front().c_str(),true);
  return 0;
}
