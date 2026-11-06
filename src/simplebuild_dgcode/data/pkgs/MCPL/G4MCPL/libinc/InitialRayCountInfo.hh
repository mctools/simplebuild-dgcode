#ifndef G4MCPL_InitialRayCountInfo_hh
#define G4MCPL_InitialRayCountInfo_hh

namespace G4MCPL {

  //Interface implemented by particle generators reading particles from MCPL
  //files (i.e. G4MCPLPlugins/MCPLGen), with information about the
  //stat:sum:initial_ray_count value of the input file. This is used by
  //G4MCPLPlugins/MCPLWriter to transfer the value to its output files, with
  //the weights written in the same convention as in the input file (i.e. the
  //weights of the input file divided by the value are the weights per initial
  //ray).

  class InitialRayCountInfo {
  public:
    virtual ~InitialRayCountInfo() = default;

    //The stat:sum:initial_ray_count value of the input file: -2 if not
    //present, -1 if not available, and otherwise the (positive) value. This
    //only reads the header of the input file, so it can be called at any time
    //after the generator is configured:
    virtual double inputInitialRayCount() const = 0;

    //The factor applied to the weights of the input file when generating
    //particles (1/initial_ray_count when normalising the weights, otherwise
    //1):
    virtual double inputWeightFactor() const = 0;

    //Whether all particles of the input file (in this process, when
    //multiprocessing) were used:
    virtual bool usedAllInputParticles() const = 0;
  };

}

#endif
