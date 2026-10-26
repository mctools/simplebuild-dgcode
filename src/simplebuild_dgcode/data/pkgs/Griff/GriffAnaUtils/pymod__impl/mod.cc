// sbldpy module used by the GriffAnaUtils Python module, providing the C++
// filters and iterators. The iterators work on the C++ reader of a Python
// GriffDataReader (identified by its handle), and write the indices of the
// selected objects into a buffer owned by the Python code, from which the
// Python objects are created.

#include "sbldpy/sbldpy.hh"
#include "GriffDataRead/ReaderHandles.hh"
#include "GriffAnaUtils/All.hh"
#include <memory>
#include <stdexcept>

namespace GriffAnaUtils_py {

  namespace GA = GriffAnaUtils;
  enum Kind : std::int64_t { TRACK = 0, SEGMENT = 1, STEP = 2 };

  const GriffDataRead::Track& getTrack( std::int64_t handle, std::int64_t itrk )
  {
    auto& dr = GriffDataRead::getReader( handle );
    if ( !dr.eventActive() || itrk < 0 || itrk >= static_cast<std::int64_t>( dr.nTracks() ) )
      throw std::out_of_range( "invalid track index" );
    return *dr.getTrack( static_cast<unsigned>( itrk ) );
  }
  const GriffDataRead::Segment& getSegment( std::int64_t handle, std::int64_t itrk, std::int64_t iseg )
  {
    const auto& trk = getTrack( handle, itrk );
    if ( iseg < 0 || iseg >= static_cast<std::int64_t>( trk.nSegments() ) )
      throw std::out_of_range( "invalid segment index" );
    return *trk.getSegment( static_cast<unsigned>( iseg ) );
  }
  const GriffDataRead::Step& getStep( std::int64_t handle, std::int64_t itrk,
                                      std::int64_t iseg, std::int64_t istep )
  {
    const auto& seg = getSegment( handle, itrk, iseg );
    if ( istep < 0 || istep >= static_cast<std::int64_t>( seg.nStepsStored() ) )
      throw std::out_of_range( "invalid step index" );
    return *seg.getStep( static_cast<unsigned>( istep ) );
  }

  //Filter of any type (reference counted, as the C++ filters):
  class Filter {
  public:
    Filter( GA::IFilter * f, Kind kind ) : m_f( f ), m_kind( kind ) { m_f->ref(); }
    Filter( const Filter& o ) : m_f( o.m_f ), m_kind( o.m_kind ) { m_f->ref(); }
    Filter& operator=( const Filter& ) = delete;
    ~Filter() { m_f->unref(); }

    std::int64_t kind() const { return m_kind; }
    void setNegated( bool b ) { m_f->setNegated( b ); }
    bool negated() const { return m_f->negated(); }
    void setEnabled( bool b ) { m_f->setEnabled( b ); }
    bool enabled() const { return m_f->enabled(); }

    template<class T>
    T& as() const
    {
      auto f = dynamic_cast<T*>( m_f );
      if ( !f )
        throw std::invalid_argument( "method not supported by filter" );
      return *f;
    }

    //Evaluate the filter (not taking negation into account) for the given
    //object of the current event:
    bool filterTrack( std::int64_t handle, std::int64_t itrk ) const
    {
      return as<GA::ITrackFilter>().filter( &getTrack( handle, itrk ) );
    }
    bool filterSegment( std::int64_t handle, std::int64_t itrk, std::int64_t iseg ) const
    {
      return as<GA::ISegmentFilter>().filter( &getSegment( handle, itrk, iseg ) );
    }
    bool filterStep( std::int64_t handle, std::int64_t itrk, std::int64_t iseg, std::int64_t istep ) const
    {
      return as<GA::IStepFilter>().filter( &getStep( handle, itrk, iseg, istep ) );
    }

  private:
    GA::IFilter * m_f;
    Kind m_kind;
  };

  //Iterator of any type, filling a buffer owned by the Python code (which
  //keeps it alive) with the number of selected objects followed by their
  //indices: track index for tracks, (track index, segment index) for
  //segments, and (track index, segment index, step index) for steps.
  class Iterator {
  public:
    Iterator( std::int64_t kind, std::int64_t handle, sbldpy::NumpyView<std::int32_t> buffer )
      : m_kind( static_cast<Kind>( kind ) ), m_handle( handle ), m_buf( buffer )
    {
      if ( m_buf.size() < 1 )
        throw std::length_error( "buffer too small" );
      auto& dr = GriffDataRead::getReader( handle );
      if ( kind == TRACK )
        m_trkIter = std::make_unique<GA::TrackIterator>( &dr );
      else if ( kind == SEGMENT )
        m_segIter = std::make_unique<GA::SegmentIterator>( &dr );
      else if ( kind == STEP )
        m_stepIter = std::make_unique<GA::StepIterator>( &dr );
      else
        throw std::invalid_argument( "invalid iterator kind" );
    }

    void addFilter( const Filter& f )
    {
      if ( f.kind() > m_kind )
        throw std::invalid_argument( "filter can not be used with this iterator" );
      if ( f.kind() == TRACK ) {
        auto& tf = f.as<GA::ITrackFilter>();
        if ( m_trkIter )
          m_trkIter->addFilter( &tf );
        else if ( m_segIter )
          m_segIter->addFilter( &tf );
        else
          m_stepIter->addFilter( &tf );
      } else if ( f.kind() == SEGMENT ) {
        auto& sf = f.as<GA::ISegmentFilter>();
        if ( m_segIter )
          m_segIter->addFilter( &sf );
        else
          m_stepIter->addFilter( &sf );
      } else {
        m_stepIter->addFilter( &f.as<GA::IStepFilter>() );
      }
    }

    //Fill the buffer for the current event:
    void fill()
    {
      try {
        actualFill();
      } catch ( std::length_error& e ) {
        m_buf[0] = 0;
        throw std::length_error( std::string( e.what() )
                                 + " (increase buffer_mb of the GriffDataReader)" );
      }
    }

  private:
    void actualFill()
    {
      auto& dr = GriffDataRead::getReader( m_handle );
      std::int32_t * out = m_buf.data() + 1;
      std::int32_t * outE = m_buf.data() + m_buf.size();
      std::int64_t n = 0;
      m_buf[0] = 0;
      if ( !dr.eventActive() )
        return;
      const GriffDataRead::Track * trkBegin = dr.trackBegin();
      auto write = [&out,outE]( std::int64_t v )
      {
        if ( out == outE )
          throw std::length_error( "Too many selected objects for the buffer of the iterator" );
        *out++ = static_cast<std::int32_t>( v );
      };
      if ( m_trkIter ) {
        m_trkIter->reset();
        while ( auto trk = m_trkIter->next() ) {
          write( trk - trkBegin );
          ++n;
        }
      } else if ( m_segIter ) {
        m_segIter->reset();
        while ( auto seg = m_segIter->next() ) {
          write( seg->getTrack() - trkBegin );
          write( seg->iSegment() );
          ++n;
        }
      } else {
        m_stepIter->reset();
        while ( auto step = m_stepIter->next() ) {
          auto seg = step->getSegment();
          write( seg->getTrack() - trkBegin );
          write( seg->iSegment() );
          write( step->iStep() );
          ++n;
        }
      }
      m_buf[0] = static_cast<std::int32_t>( n );
    }

    Kind m_kind;
    std::int64_t m_handle;
    sbldpy::NumpyView<std::int32_t> m_buf;//owned by the Python code
    std::unique_ptr<GA::TrackIterator> m_trkIter;
    std::unique_ptr<GA::SegmentIterator> m_segIter;
    std::unique_ptr<GA::StepIterator> m_stepIter;
  };

}

SBLDPY_MODULE( _impl )
{
  using namespace GriffAnaUtils_py;
  m.add_class<Filter>( "Filter" )
    .def( "kind", &Filter::kind )
    .def( "set_negated", &Filter::setNegated, { "b" } )
    .def( "negated", &Filter::negated )
    .def( "set_enabled", &Filter::setEnabled, { "b" } )
    .def( "enabled", &Filter::enabled )
    .def( "filter_track", &Filter::filterTrack )
    .def( "filter_segment", &Filter::filterSegment )
    .def( "filter_step", &Filter::filterStep )
    .def( "pdg_add_code", []( Filter& f, std::int64_t c ) { f.as<GA::TrackFilter_PDGCode>().addCode( static_cast<std::int32_t>( c ) ); } )
    .def( "pdg_set_unsigned", []( Filter& f, bool b )
    {
      auto& pf = f.as<GA::TrackFilter_PDGCode>();
      if ( b )
        pf.setUnsigned();
      else
        pf.setSigned();
    } )
    .def( "pdg_is_unsigned", []( Filter& f ) { return f.as<GA::TrackFilter_PDGCode>().isUnsigned(); } )
    .def( "charged_negative_only", []( Filter& f ) { f.as<GA::TrackFilter_Charged>().setNegativeOnly(); } )
    .def( "charged_positive_only", []( Filter& f ) { f.as<GA::TrackFilter_Charged>().setPositiveOnly(); } )
    .def( "descendant_set_ancestor", []( Filter& f, std::int64_t handle, std::int64_t itrk, bool include )
    {
      f.as<GA::TrackFilter_Descendant>().setAncestor( &getTrack( handle, itrk ), include );
    } )
    .def( "volume_set_physical", []( Filter& f, bool b )
    {
      auto& vf = f.as<GA::SegmentFilter_Volume>();
      if ( b )
        vf.setPhysical();
      else
        vf.setLogical();
    } )
    .def( "volume_is_physical", []( Filter& f ) { return f.as<GA::SegmentFilter_Volume>().isPhysical(); } )
    .def( "ekin_set", []( Filter& f, std::int64_t which, double e )
    {
      //which: 0=min start/pre, 1=max start/pre, 2=min end/post, 3=max end/post
      if ( f.kind() == SEGMENT ) {
        auto& ef = f.as<GA::SegmentFilter_EKin>();
        switch ( which ) {
        case 0: ef.setMinStartEKin( e ); return;
        case 1: ef.setMaxStartEKin( e ); return;
        case 2: ef.setMinEndEKin( e ); return;
        case 3: ef.setMaxEndEKin( e ); return;
        default: break;
        }
      } else {
        auto& ef = f.as<GA::StepFilter_EKin>();
        switch ( which ) {
        case 0: ef.setMinPreEKin( e ); return;
        case 1: ef.setMaxPreEKin( e ); return;
        case 2: ef.setMinPostEKin( e ); return;
        case 3: ef.setMaxPostEKin( e ); return;
        default: break;
        }
      }
      throw std::invalid_argument( "invalid ekin_set argument" );
    } );
  m.def( "track_filter_pdgcode", []() { return Filter( new GA::TrackFilter_PDGCode, TRACK ); } );
  m.def( "track_filter_descendant", []() { return Filter( new GA::TrackFilter_Descendant, TRACK ); } );
  m.def( "track_filter_primary", []() { return Filter( new GA::TrackFilter_Primary, TRACK ); } );
  m.def( "track_filter_charged", []() { return Filter( new GA::TrackFilter_Charged, TRACK ); } );
  m.def( "segment_filter_volume", []( const std::string& v1, const std::string& v2, const std::string& v3 )
  {
    return Filter( new GA::SegmentFilter_Volume( v1.c_str(), v2.empty() ? nullptr : v2.c_str(),
                                                 v3.empty() ? nullptr : v3.c_str() ), SEGMENT );
  } );
  m.def( "segment_filter_edep", []( double thr ) { return Filter( new GA::SegmentFilter_EnergyDeposition( thr ), SEGMENT ); } );
  m.def( "segment_filter_ekin", []() { return Filter( new GA::SegmentFilter_EKin, SEGMENT ); } );
  m.def( "step_filter_edep", []( double thr ) { return Filter( new GA::StepFilter_EnergyDeposition( thr ), STEP ); } );
  m.def( "step_filter_ekin", []() { return Filter( new GA::StepFilter_EKin, STEP ); } );
  m.add_class<Iterator>( "Iterator" )
    .def_init<std::int64_t,std::int64_t,sbldpy::NumpyView<std::int32_t>>( { "kind", "handle", "buffer" } )
    .def( "add_filter", &Iterator::addFilter, { "f" } )
    .def( "fill", &Iterator::fill );
}
