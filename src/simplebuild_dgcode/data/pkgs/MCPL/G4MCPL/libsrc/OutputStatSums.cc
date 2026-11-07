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
    struct InputInfo {
      bool present = false;
      double initial_ray_count = -1.0;
      double weight_factor = 1.0;
    };
    InputInfo& inputInfo()
    {
      static InputInfo s_info;
      return s_info;
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
  if ( key == "geant4_nevts" || key == "initial_geant4_weights"
       || key == "initial_ray_count" || key == "original_initial_ray_count" )
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

void G4MCPL::setInputInitialRayCount( double initial_ray_count, double weight_factor )
{
  if ( !( weight_factor > 0.0 ) || !std::isfinite(weight_factor) )
    throw std::runtime_error("G4MCPL: Weight factor must be positive and finite");
  if ( !( initial_ray_count > 0.0 ) || !std::isfinite(initial_ray_count) )
    initial_ray_count = -1.0;//not available
  auto& info = inputInfo();
  if ( info.present )
    throw std::runtime_error("G4MCPL: The initial_ray_count of an input file was already set");
  info.present = true;
  info.initial_ray_count = initial_ray_count;
  info.weight_factor = weight_factor;
}

bool G4MCPL::hasInputInitialRayCount() { return inputInfo().present; }
double G4MCPL::inputInitialRayCount() { return inputInfo().initial_ray_count; }
double G4MCPL::inputWeightFactor() { return inputInfo().weight_factor; }
