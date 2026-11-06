#ifndef G4MCPL_OutputStatSums_hh
#define G4MCPL_OutputStatSums_hh

#include <string>
#include <vector>
#include <functional>

//Global (per process) registry of stat:sum values to add to all MCPL files
//written by G4MCPLPlugins/MCPLWriter, and of a factor to apply to the weights
//written in those files. This is for instance used by G4MCPLPlugins/MCPLGen to
//transfer stat:sum:initial_ray_count of its input file, but it can also be
//used by other components or user code to add custom values.
//
//Since MCPL requires the same header entries in all files to merge them (e.g.
//the files of the processes of multi-process jobs), entries must be added
//before the output files are created, i.e. before the simulation starts (for
//instance when the parameters of a generator are validated, or in a pre-init
//hook).

namespace G4MCPL {

  //How a value adds up when the output files of several processes are merged
  //(MCPL adds up stat:sum values when merging files):
  enum class StatSumMode {
    PerProcess,//Each process writes its own value (e.g. a count of events).
    WholeJob   //The value describes the whole job, so only the parent process
               //writes it (the others write 0). If the value is unavailable
               //(-1) in any process, it is written as -1 in all processes
               //(which makes the merged value unavailable).
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

  //Factor applied to the Geant4 weights when writing them (default 1). This is
  //for instance used by MCPLGen, so the weights are written in the same
  //convention as in its input file, when transferring initial_ray_count:
  void setOutputWeightFactor( double );
  double outputWeightFactor();

}

#endif
