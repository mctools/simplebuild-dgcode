#include "sbldpy/sbldpy.hh"
#include "SimpleHistsUtils/Sampler.hh"
#include "SimpleHists/Hist1D.hh"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

//Compiled part of SimpleHistsUtils.Sampler (see python/Sampler.py). Since
//objects can not be passed between sbldpy modules, the histogram is passed in
//serialised form (the Sampler only uses it during initialisation).

namespace SimpleHistsUtils_py {

  std::string toString( sbldpy::NumpyView<const std::uint8_t> data )
  {
    return std::string( reinterpret_cast<const char*>( data.data() ), data.size() );
  }

  std::unique_ptr<SimpleHists::Hist1D> makeHist( sbldpy::NumpyView<const std::uint8_t> data )
  {
    return std::make_unique<SimpleHists::Hist1D>( toString( data ) );
  }

  class PySampler {
  public:
    PySampler( sbldpy::NumpyView<const std::uint8_t> serialised_hist, double scalefact )
      : m_sampler( makeHist( serialised_hist ).get(), scalefact )
    {
    }
    void reinit( sbldpy::NumpyView<const std::uint8_t> serialised_hist, double scalefact )
    {
      SimpleHists::Hist1D h( toString( serialised_hist ) );
      m_sampler.reinit( &h, scalefact );
    }
    double sample( double r ) const { return m_sampler.sample( r ); }
    sbldpy::NumpyCopy<double> sampleMany( sbldpy::NumpyView<const double> rand_vals ) const
    {
      std::vector<double> res;
      res.reserve( rand_vals.size() );
      for ( double r : rand_vals )
        res.push_back( m_sampler.sample( r ) );
      return res;
    }
  private:
    SimpleHists::Sampler m_sampler;
  };

}

SBLDPY_MODULE( _sampler )
{
  using SimpleHistsUtils_py::PySampler;
  m.add_class<PySampler>( "Sampler" )
    .def_init<sbldpy::NumpyView<const std::uint8_t>,double>( { "serialised_hist", "scalefact" } )
    .def( "reinit", &PySampler::reinit, { "serialised_hist", "scalefact" } )
    .def( "sample", &PySampler::sample, { "rand" } )
    .def( "sampleMany", &PySampler::sampleMany, { "rand_vals" } );
}
