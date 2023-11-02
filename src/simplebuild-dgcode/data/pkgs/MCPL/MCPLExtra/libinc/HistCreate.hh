#ifndef MCPLExtra_HistCreate_hh
#define MCPLExtra_HistCreate_hh

#include "SimpleHists/HistCollection.hh"

namespace MCPLExtra {

  //Create standard set of histograms, optionally limiting number of particles
  //read from the file and imposing a filter expression:
  void mcplStdHists( SimpleHists::HistCollection& hc,
                     const std::string& filename,
                     const std::string& filter_expr = "",
                     unsigned long long max_particles_load = 0 );

  //Same, but create a single histogram given by provided expression (1D hist if
  //single expression, 2D if of the form "<expr1>:<expr2>"):
  SimpleHists::HistBase * mcplHistsFromExpression(const std::string& filename,
                                                  const std::string& plot_expr = "",
                                                  const std::string& filter_expr = "",
                                                  unsigned long long max_particles_load = 0 );
}

#endif
