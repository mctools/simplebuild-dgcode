#include "sbldpy/sbldpy.hh"
#include "SimpleHistsUtils/Sampler.hh"
#include "SimpleHists/Hist1D.hh"
#include "SimpleHists/Handles.hh"
#include <cstdint>
#include <stdexcept>

//Compiled part of SimpleHistsUtils.Sampler (see python/Sampler.py). The
//histogram is passed by its handle (see SimpleHists/Handles.hh).

namespace SimpleHistsUtils_py {

  SimpleHists::Hist1D * getHist1D( std::int64_t handle )
  {
    auto h = dynamic_cast<SimpleHists::Hist1D*>( SimpleHists::getHist( handle ).get() );
    if ( !h )
      throw sbldpy::TypeError( "Sampler requires a Hist1D histogram" );
    return h;//(only used during the call)
  }

  class PySampler {
  public:
    PySampler( std::int64_t hist_handle, double scalefact )
      : m_sampler( getHist1D( hist_handle ), scalefact )
    {
    }
    void reinit( std::int64_t hist_handle, double scalefact )
    {
      m_sampler.reinit( getHist1D( hist_handle ), scalefact );
    }
    double sample( double r ) const { return m_sampler.sample( r ); }
    //Replaces random numbers with sampled values:
    void sampleMany( sbldpy::NumpyView<double> vals ) const
    {
      for ( auto& v : vals )
        v = m_sampler.sample( v );
    }
  private:
    SimpleHists::Sampler m_sampler;
  };

}

SBLDPY_MODULE( _sampler )
{
  using SimpleHistsUtils_py::PySampler;
  m.add_class<PySampler>( "Sampler" )
    .def_init<std::int64_t,double>( { "hist_handle", "scalefact" } )
    .def( "reinit", &PySampler::reinit, { "hist_handle", "scalefact" } )
    .def( "sample", &PySampler::sample, { "rand" } )
    .def( "sampleMany", &PySampler::sampleMany, { "vals" } );
}
