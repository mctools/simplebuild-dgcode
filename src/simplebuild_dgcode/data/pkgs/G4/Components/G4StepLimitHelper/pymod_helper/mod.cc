#include "sbldpy/sbldpy.hh"
#include "G4StepLimitHelper/G4StepLimitHelper.hh"
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace {
  G4int toG4Int( std::int64_t n )
  {
    if ( n < std::numeric_limits<G4int>::min() || n > std::numeric_limits<G4int>::max() )
      throw std::out_of_range("integer out of range");
    return static_cast<G4int>( n );
  }
}

SBLDPY_MODULE( helper )
{
  m.add_class<G4StepLimitHelper>("G4StepLimitHelper")
    .def_init<>()
    .def("setLimit", &G4StepLimitHelper::setLimit)
    .def("addLimit", []( G4StepLimitHelper& h, std::int64_t pdgCode, const std::string& volName, double uStepMax )
         { h.addLimit( toG4Int( pdgCode ), volName.c_str(), uStepMax ); }, { "pdgCode", "volName", "uStepMax" } )
    .def("setWorldLimit", &G4StepLimitHelper::setWorldLimit)
    .def("addWorldLimit", []( G4StepLimitHelper& h, std::int64_t pdgCode, double uStepMax )
         { h.addWorldLimit( toG4Int( pdgCode ), uStepMax ); }, { "pdgCode", "uStepMax" } );
}
