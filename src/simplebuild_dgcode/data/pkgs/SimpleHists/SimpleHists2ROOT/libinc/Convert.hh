#ifndef SimpleHists2ROOT_Convert_hh
#define SimpleHists2ROOT_Convert_hh

//Provide a SimpleHist and a name and get back a corresponding ROOT histogram
//(as close as possible):

#include <string>
class TH1D;
class TH2D;
class TH1;

namespace SimpleHists {

  class HistBase;
  class Hist1D;
  class Hist2D;
  class HistCounts;
  class HistCollection;

  //Create ROOT histogram from SimpleHist histogram. Must provide name for ROOT
  //in addition to the histogram itself:
  TH1D * convertToROOT(const Hist1D*, const std::string& root_name);
  TH2D * convertToROOT(const Hist2D*, const std::string& root_name);
  TH1D * convertToROOT(const HistCounts*, const std::string& root_name);

  //Version operating on base classes, dispatching to the appropriate of the methods above:
  TH1 * convertToROOT(const HistBase*, const std::string& root_name);

  //Create ROOT files based on SimpleHist collections:
  void convertToROOTFile( const HistCollection*,
                          const std::string& filename_root );
  void convertToROOTFile( const std::string& filename_shist,
                          const std::string& filename_root );
}

#endif
