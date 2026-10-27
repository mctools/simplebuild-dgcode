#ifndef SimpleHists_Handles_hh
#define SimpleHists_Handles_hh

#include "SimpleHists/HistBase.hh"
#include <cstdint>
#include <memory>

namespace SimpleHists {

  class HistCollection;

  //Tables of histograms and collections identified by integer handles, which
  //allow code in different Python modules (which can not pass objects between
  //each other) to share histograms and collections. For instance, the Python
  //SimpleHists module can pass the handle of a histogram to a C++ function in
  //another module, which can then use getHist(handle) to access the very same
  //histogram. The tables share the ownership of the objects while they are
  //registered. Unknown handles result in std::invalid_argument exceptions.

  std::int64_t registerHist( std::shared_ptr<HistBase> );
  std::shared_ptr<HistBase> getHist( std::int64_t handle );
  void unregisterHist( std::int64_t handle );

  std::int64_t registerCollection( std::shared_ptr<HistCollection> );
  std::shared_ptr<HistCollection> getCollection( std::int64_t handle );
  void unregisterCollection( std::int64_t handle );

}

#endif
