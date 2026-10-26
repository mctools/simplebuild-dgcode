#include "sbldpy/sbldpy.hh"
#include "SimpleHists/Handles.hh"
#include "SimpleHists/Hist1D.hh"
#include "SimpleHists/Hist2D.hh"
#include "SimpleHists/HistCounts.hh"
#include "SimpleHists/HistCollection.hh"
#include <cstdint>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

//Compiled part of the SimpleHists Python module (see python/__init__.py, which
//provides the actual Python classes). All histograms are held by objects of
//the class Hist (sharing the ownership of the C++ histogram), which provides
//the methods of all histogram types (checking the type at runtime). Similarly,
//objects of the class Collection share the ownership of a C++ HistCollection.
//Histograms and collections can be shared with other modules via handles (see
//SimpleHists/Handles.hh).

namespace SimpleHists_py {

  namespace sh = SimpleHists;
  using NV = sbldpy::NumpyView<const double>;

  class Hist {
  public:
    explicit Hist( sh::HistBase* h ) : m_h( h ) { if ( !h ) throw std::runtime_error("null histogram"); }
    explicit Hist( std::shared_ptr<sh::HistBase> h ) : m_h( std::move( h ) ) { if ( !m_h ) throw std::runtime_error("null histogram"); }
    Hist( const Hist& o ) : m_h( o.m_h ) {}//(the handle is not copied)
    Hist& operator=( const Hist& ) = delete;
    ~Hist() { if ( m_handle ) sh::unregisterHist( m_handle ); }
    //Handle for other modules (registered on first use, unregistered when this
    //object is deleted):
    std::int64_t handle()
    {
      if ( !m_handle )
        m_handle = sh::registerHist( m_h );
      return m_handle;
    }
    sh::HistBase& base() { return *m_h; }
    sh::Hist1D& h1() { return cast<sh::Hist1D>( "Hist1D" ); }
    sh::Hist2D& h2() { return cast<sh::Hist2D>( "Hist2D" ); }
    sh::HistCounts& hc() { return cast<sh::HistCounts>( "HistCounts" ); }
    const std::shared_ptr<sh::HistBase>& ptr() const { return m_h; }
  private:
    std::shared_ptr<sh::HistBase> m_h;
    std::int64_t m_handle = 0;
    template<class T>
    T& cast( const char * what )
    {
      auto p = dynamic_cast<T*>( m_h.get() );
      if ( !p )
        throw sbldpy::TypeError( std::string("Histogram is not a ") + what );
      return *p;
    }
  };

  std::string toHex( const std::string& data )
  {
    static const char * digits = "0123456789abcdef";
    std::string res;
    res.reserve( 2 * data.size() );
    for ( char ch : data ) {
      const auto uc = static_cast<unsigned char>( ch );
      res += digits[uc >> 4];
      res += digits[uc & 0xF];
    }
    return res;
  }

  std::string fromBytes( sbldpy::NumpyView<const std::uint8_t> data )
  {
    return std::string( reinterpret_cast<const char*>( data.data() ), data.size() );
  }

  unsigned toUnsigned( std::int64_t n )
  {
    if ( n < 0 || n > 0xFFFFFFFFLL )
      throw std::out_of_range("number out of range");
    return static_cast<unsigned>( n );
  }

  //Flush output from Python before printing from C++ and vice versa, to keep
  //the order of the output (the Python side flushes its stdout before calling):
  struct FlushAfter {
    ~FlushAfter() { std::fflush( stdout ); }
  };

  class Collection {
  public:
    explicit Collection( std::shared_ptr<sh::HistCollection> c ) : m_c( std::move( c ) ) {}
    Collection( const Collection& o ) : m_c( o.m_c ) {}//(the handle is not copied)
    Collection& operator=( const Collection& ) = delete;
    ~Collection() { if ( m_handle ) sh::unregisterCollection( m_handle ); }
    sh::HistCollection& c() { return *m_c; }
    //Handle for other modules (registered on first use, unregistered when this
    //object is deleted):
    std::int64_t handle()
    {
      if ( !m_handle )
        m_handle = sh::registerCollection( m_c );
      return m_handle;
    }
    std::string keys() const
    {
      std::string res;//keys separated by newlines (keys can not contain newlines)
      for ( auto& e : m_c->getHistograms() ) {
        if ( !res.empty() )
          res += '\n';
        res += e.first;
      }
      return res;
    }
  private:
    std::shared_ptr<sh::HistCollection> m_c;
    std::int64_t m_handle = 0;
  };

}

SBLDPY_MODULE( _impl )
{
  using namespace SimpleHists_py;
  using sbldpy::NumpyView;

  m.def( "hist_type_of_data", []( sbldpy::NumpyView<const std::uint8_t> d ) -> std::int64_t
  {
    return sh::histTypeOfData( fromBytes( d ) );
  } );

  m.add_class<Hist>( "Hist", "C++ histogram (see SimpleHists/python/__init__.py)" )
    //Construction:
    .def_static( "new1d", []( const std::string& title, std::int64_t nbins, double xmin, double xmax )
    {
      return Hist( new sh::Hist1D( title, toUnsigned( nbins ), xmin, xmax ) );
    } )
    .def_static( "new2d", []( const std::string& title, std::int64_t nbinsx, double xmin, double xmax,
                              std::int64_t nbinsy, double ymin, double ymax )
    {
      return Hist( new sh::Hist2D( title, toUnsigned( nbinsx ), xmin, xmax, toUnsigned( nbinsy ), ymin, ymax ) );
    } )
    .def_static( "newcounts", []( const std::string& title )
    {
      auto h = new sh::HistCounts;
      h->setTitle( title );
      return Hist( h );
    } )
    .def_static( "deserialise", []( sbldpy::NumpyView<const std::uint8_t> d )
    {
      return Hist( sh::deserialise( fromBytes( d ) ) );
    } )
    .def_static( "from_handle", []( std::int64_t handle )
    {
      //Takes over the registered histogram (sharing its ownership):
      auto h = sh::getHist( handle );
      sh::unregisterHist( handle );
      return Hist( std::move( h ) );
    } )
    .def( "handle", []( Hist& h ) { return h.handle(); } )
    //HistBase:
    .def( "same", []( Hist& h, Hist& o ) { return h.ptr() == o.ptr(); } )
    .def( "hist_type", []( Hist& h ) -> std::int64_t { return h.base().histType(); } )
    .def( "get_title", []( Hist& h ) { return h.base().getTitle(); } )
    .def( "get_xlabel", []( Hist& h ) { return h.base().getXLabel(); } )
    .def( "get_ylabel", []( Hist& h ) { return h.base().getYLabel(); } )
    .def( "get_comment", []( Hist& h ) { return h.base().getComment(); } )
    .def( "set_title", []( Hist& h, const std::string& s ) { h.base().setTitle( s ); } )
    .def( "set_xlabel", []( Hist& h, const std::string& s ) { h.base().setXLabel( s ); } )
    .def( "set_ylabel", []( Hist& h, const std::string& s ) { h.base().setYLabel( s ); } )
    .def( "set_comment", []( Hist& h, const std::string& s ) { h.base().setComment( s ); } )
    .def( "integral", []( Hist& h ) { return h.base().getIntegral(); } )
    .def( "empty", []( Hist& h ) { return h.base().empty(); } )
    .def( "dimension", []( Hist& h ) -> std::int64_t { return h.base().dimension(); } )
    .def( "serialise_hex", []( Hist& h ) { std::string s; h.base().serialise( s ); return toHex( s ); } )
    .def( "dump", []( Hist& h, bool contents, const std::string& prefix ) { FlushAfter f; h.base().dump( contents, prefix ); } )
    .def( "merge_compatible", []( Hist& h, Hist& o ) { return h.base().mergeCompatible( &o.base() ); } )
    .def( "merge", []( Hist& h, Hist& o )
    {
      if ( !h.base().mergeCompatible( &o.base() ) )
        throw std::invalid_argument("Histograms are not compatible for merging");
      h.base().merge( &o.base() );
    } )
    .def( "is_similar", []( Hist& h, Hist& o ) { return h.base().isSimilar( &o.base() ); } )
    .def( "scale", []( Hist& h, double f ) { h.base().scale( f ); } )
    .def( "norm", []( Hist& h ) { h.base().norm(); } )
    .def( "clone", []( Hist& h ) { return Hist( h.base().clone() ); } )
    .def( "reset", []( Hist& h ) { h.base().reset(); } )
    //Hist1D:
    .def( "h1_nbins", []( Hist& h ) -> std::int64_t { return h.h1().getNBins(); } )
    .def( "h1_bincontent", []( Hist& h, std::int64_t i ) { return h.h1().getBinContent( toUnsigned( i ) ); } )
    .def( "h1_binerror", []( Hist& h, std::int64_t i ) { return h.h1().getBinError( toUnsigned( i ) ); } )
    .def( "h1_bincenter", []( Hist& h, std::int64_t i ) { return h.h1().getBinCenter( toUnsigned( i ) ); } )
    .def( "h1_binlower", []( Hist& h, std::int64_t i ) { return h.h1().getBinLower( toUnsigned( i ) ); } )
    .def( "h1_binupper", []( Hist& h, std::int64_t i ) { return h.h1().getBinUpper( toUnsigned( i ) ); } )
    .def( "h1_binwidth", []( Hist& h ) { return h.h1().getBinWidth(); } )
    .def( "h1_maxcontent", []( Hist& h ) { return h.h1().getMaxContent(); } )
    .def( "h1_underflow", []( Hist& h ) { return h.h1().getUnderflow(); } )
    .def( "h1_overflow", []( Hist& h ) { return h.h1().getOverflow(); } )
    .def( "h1_minfilled", []( Hist& h ) { return h.h1().getMinFilled(); } )
    .def( "h1_maxfilled", []( Hist& h ) { return h.h1().getMaxFilled(); } )
    .def( "h1_xmin", []( Hist& h ) { return h.h1().getXMin(); } )
    .def( "h1_xmax", []( Hist& h ) { return h.h1().getXMax(); } )
    .def( "h1_mean", []( Hist& h ) { return h.h1().getMean(); } )
    .def( "h1_rms", []( Hist& h ) { return h.h1().getRMS(); } )
    .def( "h1_rms2", []( Hist& h ) { return h.h1().getRMSSquared(); } )
    .def( "h1_valuetobin", []( Hist& h, double v ) -> std::int64_t { return h.h1().valueToBin( v ); } )
    .def( "h1_fill", []( Hist& h, double v ) { h.h1().fill( v ); } )
    .def( "h1_fillw", []( Hist& h, double v, double w ) { h.h1().fill( v, w ); } )
    .def( "h1_fillmany", []( Hist& h, NV v )
    {
      h.h1().fillMany( v.data(), toUnsigned( static_cast<std::int64_t>( v.size() ) ) );
    } )
    .def( "h1_fillmanyw", []( Hist& h, NV v, NV w )
    {
      if ( v.size() != w.size() )
        throw std::invalid_argument("Value and weights buffers must have equal length");
      h.h1().fillMany( v.data(), w.data(), toUnsigned( static_cast<std::int64_t>( v.size() ) ) );
    } )
    .def( "h1_filln", []( Hist& h, std::int64_t n, double v )
    {
      if ( n < 0 )
        throw std::out_of_range("number out of range");
      h.h1().fillN( static_cast<unsigned long>( n ), v );
    } )
    .def( "h1_contents", []( Hist& h )
    {
      auto& hh = h.h1();
      return NumpyView<const double>( hh.rawContents(), hh.getNBins() );
    } )
    .def( "h1_errorssquared", []( Hist& h )
    {
      auto& hh = h.h1();
      return NumpyView<const double>( hh.rawErrorsSquared(), hh.getNBins() );
    } )
    .def( "h1_seterrorsbycontent", []( Hist& h ) { h.h1().setErrorsByContent(); } )
    .def( "h1_canrebin", []( Hist& h, std::int64_t n ) { return h.h1().canRebin( toUnsigned( n ) ); } )
    .def( "h1_rebin", []( Hist& h, std::int64_t n )
    {
      auto& hh = h.h1();
      if ( !hh.canRebin( toUnsigned( n ) ) )
        throw std::invalid_argument("Histogram can not be rebinned to the requested number of bins");
      hh.rebin( toUnsigned( n ) );
    } )
    .def( "h1_resetandrebin", []( Hist& h, std::int64_t n, double xmin, double xmax )
    {
      h.h1().resetAndRebin( toUnsigned( n ), xmin, xmax );
    } )
    .def( "h1_percentilebin", []( Hist& h, double p ) -> std::int64_t { return h.h1().getPercentileBin( p ); } )
    .def( "h1_binsum", []( Hist& h, std::int64_t b1, std::int64_t b2 )
    {
      if ( b1 < -1 || b2 < -1 || b1 > 0x7FFFFFFFLL || b2 > 0x7FFFFFFFLL )
        throw std::out_of_range("bin out of range");
      return h.h1().getBinSum( static_cast<int>( b1 ), static_cast<int>( b2 ) );
    } )
    //Hist2D:
    .def( "h2_nbinsx", []( Hist& h ) -> std::int64_t { return h.h2().getNBinsX(); } )
    .def( "h2_nbinsy", []( Hist& h ) -> std::int64_t { return h.h2().getNBinsY(); } )
    .def( "h2_bincontent", []( Hist& h, std::int64_t ix, std::int64_t iy )
    {
      return h.h2().getBinContent( toUnsigned( ix ), toUnsigned( iy ) );
    } )
    .def( "h2_bincenterx", []( Hist& h, std::int64_t i ) { return h.h2().getBinCenterX( toUnsigned( i ) ); } )
    .def( "h2_binlowerx", []( Hist& h, std::int64_t i ) { return h.h2().getBinLowerX( toUnsigned( i ) ); } )
    .def( "h2_binupperx", []( Hist& h, std::int64_t i ) { return h.h2().getBinUpperX( toUnsigned( i ) ); } )
    .def( "h2_bincentery", []( Hist& h, std::int64_t i ) { return h.h2().getBinCenterY( toUnsigned( i ) ); } )
    .def( "h2_binlowery", []( Hist& h, std::int64_t i ) { return h.h2().getBinLowerY( toUnsigned( i ) ); } )
    .def( "h2_binuppery", []( Hist& h, std::int64_t i ) { return h.h2().getBinUpperY( toUnsigned( i ) ); } )
    .def( "h2_binwidthx", []( Hist& h ) { return h.h2().getBinWidthX(); } )
    .def( "h2_binwidthy", []( Hist& h ) { return h.h2().getBinWidthY(); } )
    .def( "h2_underflowx", []( Hist& h ) { return h.h2().getUnderflowX(); } )
    .def( "h2_overflowx", []( Hist& h ) { return h.h2().getOverflowX(); } )
    .def( "h2_underflowy", []( Hist& h ) { return h.h2().getUnderflowY(); } )
    .def( "h2_overflowy", []( Hist& h ) { return h.h2().getOverflowY(); } )
    .def( "h2_minfilledx", []( Hist& h ) { return h.h2().getMinFilledX(); } )
    .def( "h2_maxfilledx", []( Hist& h ) { return h.h2().getMaxFilledX(); } )
    .def( "h2_minfilledy", []( Hist& h ) { return h.h2().getMinFilledY(); } )
    .def( "h2_maxfilledy", []( Hist& h ) { return h.h2().getMaxFilledY(); } )
    .def( "h2_xmin", []( Hist& h ) { return h.h2().getXMin(); } )
    .def( "h2_xmax", []( Hist& h ) { return h.h2().getXMax(); } )
    .def( "h2_ymin", []( Hist& h ) { return h.h2().getYMin(); } )
    .def( "h2_ymax", []( Hist& h ) { return h.h2().getYMax(); } )
    .def( "h2_meanx", []( Hist& h ) { return h.h2().getMeanX(); } )
    .def( "h2_meany", []( Hist& h ) { return h.h2().getMeanY(); } )
    .def( "h2_rmsx", []( Hist& h ) { return h.h2().getRMSX(); } )
    .def( "h2_rmsy", []( Hist& h ) { return h.h2().getRMSY(); } )
    .def( "h2_rms2x", []( Hist& h ) { return h.h2().getRMSSquaredX(); } )
    .def( "h2_rms2y", []( Hist& h ) { return h.h2().getRMSSquaredY(); } )
    .def( "h2_covariance", []( Hist& h ) { return h.h2().getCovariance(); } )
    .def( "h2_correlation", []( Hist& h ) { return h.h2().getCorrelation(); } )
    .def( "h2_valuetobinx", []( Hist& h, double v ) -> std::int64_t { return h.h2().valueToBinX( v ); } )
    .def( "h2_valuetobiny", []( Hist& h, double v ) -> std::int64_t { return h.h2().valueToBinY( v ); } )
    .def( "h2_fill", []( Hist& h, double x, double y ) { h.h2().fill( x, y ); } )
    .def( "h2_fillw", []( Hist& h, double x, double y, double w ) { h.h2().fill( x, y, w ); } )
    .def( "h2_fillmany", []( Hist& h, NV x, NV y )
    {
      if ( x.size() != y.size() )
        throw std::invalid_argument("X and Y value buffers must have equal length");
      h.h2().fillMany( x.data(), y.data(), toUnsigned( static_cast<std::int64_t>( x.size() ) ) );
    } )
    .def( "h2_fillmanyw", []( Hist& h, NV x, NV y, NV w )
    {
      if ( x.size() != y.size() )
        throw std::invalid_argument("X and Y value buffers must have equal length");
      if ( w.size() != x.size() )
        throw std::invalid_argument("Value and weights buffers must have equal length");
      h.h2().fillMany( x.data(), y.data(), w.data(), toUnsigned( static_cast<std::int64_t>( x.size() ) ) );
    } )
    .def( "h2_contents", []( Hist& h )
    {
      auto& hh = h.h2();
      return NumpyView<const double>( hh.rawContents(), std::size_t( hh.getNBinsX() ) * hh.getNBinsY() );
    } )
    //HistCounts:
    .def( "hc_addcounter", []( Hist& h, const std::string& label, const std::string& display )
    {
      //Returns the actual label (labels are normalised):
      return h.hc().addCounter( label, display ).getLabel();
    } )
    .def( "hc_hascounter", []( Hist& h, const std::string& label ) { return h.hc().hasCounter( label ); } )
    .def( "hc_ncounters", []( Hist& h ) -> std::int64_t { return h.hc().nCounters(); } )
    .def( "hc_labels", []( Hist& h )
    {
      std::list<sh::HistCounts::Counter> l;
      h.hc().getCounters( l );
      std::string res;//labels separated by newlines (labels can not contain newlines)
      for ( auto& c : l ) {
        if ( !res.empty() )
          res += '\n';
        res += c.getLabel();
      }
      return res;
    } )
    .def( "hc_maxcontent", []( Hist& h ) { return h.hc().getMaxContent(); } )
    .def( "hc_seterrorsbycontent", []( Hist& h ) { h.hc().setErrorsByContent(); } )
    .def( "hc_sortbylabels", []( Hist& h ) { h.hc().sortByLabels(); } )
    .def( "hc_sortbydisplaylabels", []( Hist& h ) { h.hc().sortByDisplayLabels(); } )
    .def( "hc_value", []( Hist& h, const std::string& l ) { return h.hc().getCounter( l ).getValue(); } )
    .def( "hc_error", []( Hist& h, const std::string& l ) { return h.hc().getCounter( l ).getError(); } )
    .def( "hc_errorsquared", []( Hist& h, const std::string& l ) { return h.hc().getCounter( l ).getErrorSquared(); } )
    .def( "hc_displaylabel", []( Hist& h, const std::string& l ) { return h.hc().getCounter( l ).getDisplayLabel(); } )
    .def( "hc_setdisplaylabel", []( Hist& h, const std::string& l, const std::string& s )
    {
      h.hc().getCounter( l ).setDisplayLabel( s );
    } )
    .def( "hc_comment", []( Hist& h, const std::string& l ) { return h.hc().getCounter( l ).getComment(); } )
    .def( "hc_setcomment", []( Hist& h, const std::string& l, const std::string& s )
    {
      h.hc().getCounter( l ).setComment( s );
    } )
    .def( "hc_add", []( Hist& h, const std::string& l, double w ) { auto c = h.hc().getCounter( l ); c += w; } )
    .def( "hc_addcounter_from", []( Hist& h, const std::string& l, Hist& o, const std::string& ol )
    {
      auto c = h.hc().getCounter( l );
      c += o.hc().getCounter( ol );
    } )
    ;

  m.add_class<Collection>( "Collection", "C++ histogram collection (see SimpleHists/python/__init__.py)" )
    .def_static( "load", []( const std::string& filename )
    {
      FlushAfter f;
      return Collection( std::make_shared<sh::HistCollection>( filename ) );
    } )
    .def_static( "create", []() { return Collection( std::make_shared<sh::HistCollection>() ); } )
    .def_static( "from_handle", []( std::int64_t handle )
    {
      //Takes over the registered collection (sharing its ownership):
      auto c = sh::getCollection( handle );
      sh::unregisterCollection( handle );
      return Collection( std::move( c ) );
    } )
    .def( "handle", &Collection::handle )
    .def( "keys", &Collection::keys )
    .def( "size", []( Collection& c ) -> std::int64_t { return c.c().getHistograms().size(); } )
    .def( "has_key", []( Collection& c, const std::string& key ) { return c.c().hasKey( key ); } )
    .def( "get_key", []( Collection& c, Hist& h ) { return c.c().getKey( &h.base() ); } )
    .def( "hist", []( Collection& c, const std::string& key ) { FlushAfter f; return Hist( c.c().histPtr( key ) ); } )
    .def( "book1d", []( Collection& c, const std::string& title, std::int64_t nbins, double xmin, double xmax,
                        const std::string& key )
    {
      FlushAfter f;
      c.c().book1D( title, toUnsigned( nbins ), xmin, xmax, key );
      return Hist( c.c().histPtr( key ) );
    } )
    .def( "book2d", []( Collection& c, const std::string& title, std::int64_t nbinsx, double xmin, double xmax,
                        std::int64_t nbinsy, double ymin, double ymax, const std::string& key )
    {
      FlushAfter f;
      c.c().book2D( title, toUnsigned( nbinsx ), xmin, xmax, toUnsigned( nbinsy ), ymin, ymax, key );
      return Hist( c.c().histPtr( key ) );
    } )
    .def( "bookcounts", []( Collection& c, const std::string& title, const std::string& key )
    {
      FlushAfter f;
      c.c().bookCounts( title, key );
      return Hist( c.c().histPtr( key ) );
    } )
    .def( "add", []( Collection& c, Hist& h, const std::string& key ) { FlushAfter f; c.c().add( h.ptr(), key ); } )
    .def( "remove", []( Collection& c, const std::string& key ) { FlushAfter f; return Hist( c.c().remove( key ) ); } )
    .def( "save", []( Collection& c, const std::string& fn, bool allowOverwrite ) { FlushAfter f; c.c().saveToFile( fn, allowOverwrite ); } )
    .def( "merge", []( Collection& c, Collection& o ) { FlushAfter f; c.c().merge( &o.c() ); } )
    .def( "merge_file", []( Collection& c, const std::string& fn ) { FlushAfter f; c.c().merge( fn ); } )
    .def( "is_similar", []( Collection& c, Collection& o ) { return c.c().isSimilar( &o.c() ); } )
    ;
}
