#ifndef G4MCPL_OutputStatSums_hh
#define G4MCPL_OutputStatSums_hh

#include <string>
#include <vector>
#include <functional>

//Global (per process) registry of information for the MCPL files written by
//G4MCPLPlugins/MCPLWriter:
//
//  * stat:sum values to add to all its output files (e.g. from user code).
//  * the stat:sum:initial_ray_count of an MCPL input file (set by
//    G4MCPLPlugins/MCPLGen), which MCPLWriter records in its output files.
//
//Since MCPL requires the same header entries in all files to merge them (e.g.
//the files of the processes of multi-process jobs), the information must be
//provided before the output files are created, i.e. before the simulation
//starts (for instance when the parameters of a generator are validated, or in
//a pre-init hook).

namespace G4MCPL {

  //How a value adds up when the output files of several processes are merged
  //(MCPL adds up stat:sum values when merging files):
  enum class StatSumMode {
    PerProcess,//Each process writes its own value (e.g. a count of events).
    WholeJob   //The value describes the whole job, so each of the N processes
               //writes the value divided by N (the merged file then gets the
               //value). An unavailable value (-1) is written as -1.
  };

  //Add a stat:sum entry. The function is evaluated when each output file is
  //closed, and must return a non-negative value, or -1 if the value is not
  //available. The key must be a valid MCPL stat:sum key, which is not already
  //added (or used by MCPLWriter itself):
  void addOutputStatSum( const std::string& key,
                         std::function<double()> value,
                         StatSumMode mode );

  struct OutputStatSum {
    std::string key;
    std::function<double()> value;
    StatSumMode mode;
  };
  const std::vector<OutputStatSum>& outputStatSums();

  //The stat:sum:initial_ray_count value of an MCPL input file (positive, or -1
  //if not available), and the factor which was applied to the weights of the
  //input file when generating the particles (1/initial_ray_count when
  //normalising, otherwise 1). Set by MCPLGen:
  void setInputInitialRayCount( double initial_ray_count, double weight_factor );
  bool hasInputInitialRayCount();
  double inputInitialRayCount();
  double inputWeightFactor();

}

#endif
