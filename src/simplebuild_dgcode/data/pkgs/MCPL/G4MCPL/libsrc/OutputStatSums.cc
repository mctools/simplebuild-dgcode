#include "G4MCPL/OutputStatSums.hh"
#include <stdexcept>
#include <cmath>

namespace G4MCPL {
  namespace {
    std::vector<OutputStatSum>& registry()
    {
      static std::vector<OutputStatSum> s_registry;
      return s_registry;
    }
    double& weightFactor()
    {
      static double s_factor = 1.0;
      return s_factor;
    }
  }
}

void G4MCPL::addOutputStatSum( const std::string& key,
                               std::function<double()> value,
                               StatSumMode mode )
{
  //(the remaining requirements on keys are checked by MCPL when the files are
  //created):
  if ( key.empty() || key.size() > 64 || key.find(':') != std::string::npos )
    throw std::runtime_error("G4MCPL: Invalid stat:sum key (must be non-empty, at"
                             " most 64 characters, and not contain colons): \""+key+"\"");
  if ( key == "geant4_nevts" || key == "initial_geant4_weights" )
    throw std::runtime_error("G4MCPL: The stat:sum key \""+key+"\" is reserved for MCPLWriter");
  for ( auto& e : registry() )
    if ( e.key == key )
      throw std::runtime_error("G4MCPL: The stat:sum key \""+key+"\" was already added");
  if ( !value )
    throw std::runtime_error("G4MCPL: No function given for stat:sum key \""+key+"\"");
  registry().push_back( OutputStatSum{ key, std::move(value), mode } );
}

const std::vector<G4MCPL::OutputStatSum>& G4MCPL::outputStatSums()
{
  return registry();
}

void G4MCPL::setOutputWeightFactor( double f )
{
  if ( !( f > 0.0 ) || !std::isfinite(f) )
    throw std::runtime_error("G4MCPL: Output weight factor must be positive and finite");
  weightFactor() = f;
}

double G4MCPL::outputWeightFactor()
{
  return weightFactor();
}
