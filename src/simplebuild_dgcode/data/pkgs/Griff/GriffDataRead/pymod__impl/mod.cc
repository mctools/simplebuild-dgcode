// sbldpy module used by the GriffDataRead Python module (see
// python/_reader.py). All reading of files is done here (by a C++
// GriffDataReader), and the data of the current event is unpacked into a
// buffer owned by the Python code, where it is decoded. The shared data of the
// files (volume names, materials, particle definitions, metadata, ...) is
// accessed with the lookup methods.

#include "sbldpy/sbldpy.hh"
#include "GriffDataRead/GriffDataReader.hh"
#include "GriffDataRead/ReaderHandles.hh"
#include <cstring>
#include <memory>
#include <stdexcept>

namespace GriffDataRead {

  //Access to private data of the reader (declared as friend in the headers):
  struct ExpertAccess {
    static unsigned fileIndex( const GriffDataReader& dr ) { return dr.m_fileIdx; }

    //Raw data of the objects (in the event buffer):
    static const char * rawData( const Track& t ) { return t.m_data; }
    static const char * rawData( const Segment& s ) { return s.m_data; }
    static const char * rawData( const Step& s ) { return s.m_data; }

    static const std::string& dbString( GriffDataReader& dr, std::int64_t subsect, std::int64_t idx )
    {
      namespace F = GriffFormat;
      const auto i = static_cast<EvtFile::index_type>( idx );
      switch ( subsect ) {
      case F::Format::subsectid_volnames: return dr.m_dbVolNames.getString( i );
      case F::Format::subsectid_materialnames: return dr.m_dbMaterialNames.getString( i );
      case F::Format::subsectid_elementnames: return dr.m_dbElementNames.getString( i );
      case F::Format::subsectid_isotopenames: return dr.m_dbIsotopeNames.getString( i );
      case F::Format::subsectid_procnames: return dr.m_dbProcNames.getString( i );
      case F::Format::subsectid_pdgnames: return dr.m_dbPDGNames.getString( i );
      case F::Format::subsectid_pdgtypes: return dr.m_dbPDGTypes.getString( i );
      case F::Format::subsectid_pdgsubtypes: return dr.m_dbPDGSubTypes.getString( i );
      default:
        throw std::invalid_argument( "invalid string database id" );
      }
    }

    //All volume names loaded so far, starting at the given index (separated
    //by null characters):
    static std::string volumeNames( GriffDataReader& dr, std::int64_t start )
    {
      std::string res;
      const std::size_t n = dr.m_dbVolNames.size();
      for ( std::size_t i = static_cast<std::size_t>( start ); i < n; ++i ) {
        if ( i != static_cast<std::size_t>( start ) )
          res += '\0';
        res += dr.m_dbVolNames.getString( static_cast<EvtFile::index_type>( i ) );
      }
      return res;
    }

    //All touchables loaded so far, starting at the given index. For each, the
    //depth is followed by the copy number and indices of the volume name,
    //physical volume name and material for each depth:
    static sbldpy::NumpyCopy<std::int64_t> touchables( GriffDataReader& dr, std::int64_t start )
    {
      std::vector<std::int64_t> res;
      const std::size_t n = dr.m_dbTouchables.size();
      for ( std::size_t i = static_cast<std::size_t>( start ); i < n; ++i ) {
        const auto& t = dr.m_dbTouchables.getEntry( static_cast<EvtFile::index_type>( i ) );
        res.push_back( static_cast<std::int64_t>( t.m_info.size() ) );
        for ( const auto& d : t.m_info ) {
          res.push_back( d.copyNbr );
          res.push_back( d.volNameIdx );
          res.push_back( d.physVolNameIdx );
          res.push_back( d.materialIdx );
        }
      }
      return res;
    }

    using MaterialInfo = std::tuple<std::int64_t,double,double,double,double,double,double,std::int64_t>;
    static MaterialInfo material( GriffDataReader& dr, std::int64_t idx )
    {
      const auto& m = dr.m_dbMaterials.getEntry( static_cast<EvtFile::index_type>( idx ) );
      return { m.m_nameIdx, m.m_density, m.m_temperature, m.m_pressure, m.m_radlen,
               m.m_nuclearinterlength, m.m_ion_meanexcitationenergy, m.m_state };
    }

    //Pairs of (fraction, database index) of the elements in a material:
    static sbldpy::NumpyCopy<double> materialElements( GriffDataReader& dr, std::int64_t idx )
    {
      const auto& m = dr.m_dbMaterials.getEntry( static_cast<EvtFile::index_type>( idx ) );
      std::vector<double> res;
      for ( const auto& e : m.m_elements ) {
        res.push_back( e.fraction );
        res.push_back( static_cast<double>( e.index ) );
      }
      return res;
    }

    using ElementInfo = std::tuple<std::int64_t,std::int64_t,double,double,double,bool>;
    static ElementInfo element( GriffDataReader& dr, std::int64_t idx )
    {
      const auto& e = dr.m_dbElements.getEntry( static_cast<EvtFile::index_type>( idx ) );
      return { e.m_nameIdx, e.m_symbolIdx, e.m_Z, e.m_N, e.m_A, e.m_naturalabundances };
    }

    //Pairs of (relative abundance, database index) of the isotopes in an element:
    static sbldpy::NumpyCopy<double> elementIsotopes( GriffDataReader& dr, std::int64_t idx )
    {
      const auto& e = dr.m_dbElements.getEntry( static_cast<EvtFile::index_type>( idx ) );
      std::vector<double> res;
      for ( const auto& i : e.m_isotopes ) {
        res.push_back( i.relativeabundance );
        res.push_back( static_cast<double>( i.index ) );
      }
      return res;
    }

    using IsotopeInfo = std::tuple<std::int64_t,std::int64_t,std::int64_t,double,std::int64_t>;
    static IsotopeInfo isotope( GriffDataReader& dr, std::int64_t idx )
    {
      const auto& i = dr.m_dbIsotopes.getEntry( static_cast<EvtFile::index_type>( idx ) );
      return { i.m_nameIdx, i.Z(), i.N(), i.A(), i.m() };
    }

    using PDefInfo = std::tuple<double,double,double,double,std::int64_t,std::int64_t,std::int64_t,
                                std::int64_t,std::int64_t,std::int64_t,double,std::int64_t,bool,bool>;
    static PDefInfo particleDefinition( GriffDataReader& dr, std::int64_t pdgcode )
    {
      const auto& p = *dr.m_dbPDGCodes.getParticleDefinition( static_cast<int>( pdgcode ) );
      return { p.mass, p.width, p.charge, p.lifeTime, p.nameIdx, p.typeIdx, p.subTypeIdx,
               p.pdgcode, p.atomicNumber, p.atomicMass, p.magneticMoment, p.spin_halfs,
               p.stable != 0, p.shortLived != 0 };
    }

    //The metadata as (key, value) pairs, each string preceded by its length
    //as a 4 byte integer (values can contain binary data):
    static sbldpy::NumpyCopy<std::uint8_t> metaData( GriffDataReader& dr, std::int64_t mdidx )
    {
      const auto& m = dr.m_dbMetaData.getEntry( static_cast<EvtFile::index_type>( mdidx ) ).getMap( &dr );
      std::vector<std::uint8_t> res;
      auto add = [&res]( const std::string& s )
      {
        const std::uint32_t n = static_cast<std::uint32_t>( s.size() );
        const auto pos = res.size();
        res.resize( pos + sizeof(n) + n );
        std::memcpy( res.data() + pos, &n, sizeof(n) );
        std::memcpy( res.data() + pos + sizeof(n), s.data(), n );
      };
      for ( const auto& kv : m ) {
        add( kv.first );
        add( kv.second );
      }
      return res;
    }
  };

}

namespace GriffDataRead_py {

  //Layout of the header at the start of the buffer (64 bit integers), which is
  //updated whenever the reader moves to another event or loads data. The
  //event data follows the header:
  enum : std::size_t { HDR_ACTIVE, HDR_BRIEF_OFFSET, HDR_BRIEF_SIZE, HDR_FULL_OFFSET,
                       HDR_FULL_SIZE, HDR_RUN, HDR_EVENT, HDR_FILE_INDEX, HDR_EVENT_INDEX,
                       HDR_LOOP_COUNT, HDR_CHECKSUM, HDR_VERSION, HDR_SERIAL, HDR_NENTRIES };
  constexpr std::size_t HDR_BYTES = 128;
  static_assert( HDR_NENTRIES * sizeof(std::int64_t) <= HDR_BYTES );

  using GriffDataRead::ExpertAccess;

  //Calls f(), adding a hint to the error raised for events which do not fit in
  //the buffer:
  template<class TFct>
  auto withBufferHint( TFct f ) -> decltype( f() )
  {
    try {
      return f();
    } catch ( std::length_error& e ) {
      throw std::length_error( std::string( e.what() )
                               + " (increase buffer_mb of the GriffDataReader)" );
    }
  }

  //Layout of the tables of decoded data, written in a separate buffer when
  //requested. The header (64 bit integers) gives the number of tracks and the
  //offset of their records, the number of segments and the offset of their
  //records, and the number of steps (-1 if not decoded), the offset of their
  //records and the offset of the (first step index, number of steps) pairs of
  //the segments:
  enum : std::size_t { TBL_NTRACKS, TBL_TRACKS, TBL_NSEGMENTS, TBL_SEGMENTS,
                       TBL_NSTEPS, TBL_STEPS, TBL_SEGSTEPS, TBL_NENTRIES };
  constexpr std::size_t TBL_HDR_BYTES = 64;
  static_assert( TBL_NENTRIES * sizeof(std::int64_t) <= TBL_HDR_BYTES );
  //Records (all little endian, see python/_reader.py):
  // * Track (56 bytes): The 28 bytes of the stored track record (id, pdg code,
  //   weight, creator process, parent id, number of segments, number of
  //   daughters), followed by the index of its first segment, the offset of
  //   its daughter list in the brief data, the track index (4 byte unsigned
  //   integers), and the start time and kinetic energy (doubles).
  // * Segment (64 bytes): The 48 bytes of the stored segment record including
  //   its end time and kinetic energy, followed by the track index, the
  //   segment index on the track, and the index of the segment in the event (4
  //   byte unsigned integers), and 4 bytes of padding.
  // * Step (168 bytes): The 152 bytes of the stored step record including its
  //   post-step point, followed by the track index, segment index and step
  //   index (4 byte unsigned integers), and 4 bytes of padding.
  constexpr std::size_t REC_TRACK = 56, REC_SEGMENT = 64, REC_STEP = 168;
  constexpr std::size_t RAW_TRACK = 28, RAW_SEGMENT = 48, RAW_STEP = 152;

  class Reader {
  public:
    Reader( const std::string& files, std::int64_t nloops, sbldpy::NumpyView<std::uint8_t> buffer,
            sbldpy::NumpyView<std::uint8_t> tables )
      : m_buf( buffer ), m_tbl( tables )
    {
      if ( m_buf.size() < HDR_BYTES || m_tbl.size() < TBL_HDR_BYTES )
        throw std::length_error( "buffer too small" );
      std::vector<std::string> filelist;
      std::size_t pos = 0;
      while ( pos <= files.size() ) {
        auto next = files.find( '\n', pos );
        if ( next == std::string::npos )
          next = files.size();
        filelist.push_back( files.substr( pos, next - pos ) );
        pos = next + 1;
      }
      if ( nloops < 0 )
        throw std::invalid_argument( "nloops must not be negative" );
      m_dr = std::make_unique<GriffDataReader>( filelist, static_cast<unsigned>( nloops ) );
      //Consistency of setups is checked in the Python code:
      m_dr->allowSetupChange();
      m_dr->setExternalEventBuffer( reinterpret_cast<char*>( m_buf.data() ), m_buf.size(), HDR_BYTES );
      m_handle = GriffDataRead::registerReader( m_dr.get() );
      withBufferHint( [this]{ updateHeader(); } );
    }
    ~Reader() { GriffDataRead::unregisterReader( m_handle ); }
    Reader( const Reader& ) = delete;
    Reader& operator=( const Reader& ) = delete;

    std::int64_t handle() const { return m_handle; }

    //Navigation (updating the header). NB: Callbacks of iterators might load
    //event data:
    bool goToFirstEvent() { return withBufferHint( [this]{ return update( m_dr->goToFirstEvent() ); } ); }
    bool goToNextEvent() { return withBufferHint( [this]{ return update( m_dr->goToNextEvent() ); } ); }
    bool goToNextFile() { return withBufferHint( [this]{ return update( m_dr->goToNextFile() ); } ); }
    bool skipEvents( std::int64_t n )
    {
      if ( n < 0 )
        throw std::invalid_argument( "number of events to skip must not be negative" );
      return withBufferHint( [this,n]{ return update( m_dr->skipEvents( static_cast<unsigned>( n ) ) ); } );
    }
    bool loopEvents() { return withBufferHint( [this]{ return update( m_dr->loopEvents() ); } ); }
    bool seekEventByIndexInCurrentFile( std::int64_t idx )
    {
      if ( idx < 0 )
        return false;
      return withBufferHint( [this,idx]{ return update( m_dr->seekEventByIndexInCurrentFile( static_cast<unsigned>( idx ) ) ); } );
    }

    //Unpack the full data of the current event in the buffer (the brief data
    //is always unpacked):
    void loadFullData()
    {
      checkActive();
      auto fr = m_dr->getRawFileReader();
      if ( fr->nBytesFullDataOnDisk() && !withBufferHint( [fr]{ return fr->getFullData(); } ) )
        throw std::runtime_error( "Problems encountered while reading event data" );
      hdr()[HDR_FULL_SIZE] = fr->nBytesFullData();
    }

    bool verifyEventDataIntegrity()
    {
      checkActive();
      const bool res = withBufferHint( [this]{ return m_dr->verifyEventDataIntegrity(); } );
      hdr()[HDR_FULL_SIZE] = m_dr->getRawFileReader()->nBytesFullData();//(now loaded)
      return res;
    }

    //Write the tables of tracks and segments of the current event:
    void decodeTracks()
    {
      checkActive();
      withBufferHint( [this]{ actualDecodeTracks(); } );
    }

    //Write the table of steps of the current event (requires the tables of
    //tracks and segments):
    void decodeSteps()
    {
      checkActive();
      withBufferHint( [this]{ actualDecodeSteps(); } );
    }

    //Lookups in the shared data of the current file:
    std::string dbString( std::int64_t subsect, std::int64_t idx )
    {
      return ExpertAccess::dbString( *m_dr, subsect, idx );
    }
    GriffDataReader& dr() { return *m_dr; }

  private:
    sbldpy::NumpyView<std::uint8_t> m_buf;//owned by the Python code
    sbldpy::NumpyView<std::uint8_t> m_tbl;//owned by the Python code
    std::unique_ptr<GriffDataReader> m_dr;

    std::int64_t * tbl() { return reinterpret_cast<std::int64_t*>( m_tbl.data() ); }
    char * tblCheck( std::size_t offset, std::size_t n )
    {
      if ( offset > m_tbl.size() || n > m_tbl.size() - offset )
        throw std::length_error( "Event data does not fit in the buffer" );
      return reinterpret_cast<char*>( m_tbl.data() ) + offset;
    }
    static void put32( char * dest, std::uint32_t v ) { std::memcpy( dest, &v, sizeof(v) ); }

    void actualDecodeTracks()
    {
      auto t = tbl();
      std::memset( t, 0, TBL_HDR_BYTES );
      t[TBL_NSTEPS] = -1;
      const std::size_t ntrk = m_dr->nTracks();
      const GriffDataRead::Track * trkBegin = m_dr->trackBegin();
      const GriffDataRead::Segment * seg0 = ntrk ? trkBegin->segmentBegin() : nullptr;
      std::size_t nseg = 0;
      for ( std::size_t i = 0; i < ntrk; ++i )
        nseg += trkBegin[i].nSegments();
      const char * brief = m_dr->getRawFileReader()->getBriefData();
      const std::size_t trkoff = TBL_HDR_BYTES;
      const std::size_t segoff = trkoff + REC_TRACK * ntrk;
      char * trec = tblCheck( trkoff, REC_TRACK * ntrk + REC_SEGMENT * nseg );
      char * srec = trec + REC_TRACK * ntrk;
      for ( std::size_t i = 0; i < ntrk; ++i ) {
        const auto& trk = trkBegin[i];
        const char * raw = ExpertAccess::rawData( trk );
        const auto segB = trk.segmentBegin();
        const auto segE = trk.segmentEnd();
        std::memcpy( trec, raw, RAW_TRACK );
        put32( trec + 28, static_cast<std::uint32_t>( segB - seg0 ) );
        put32( trec + 32, static_cast<std::uint32_t>( raw + RAW_TRACK - brief ) );
        put32( trec + 36, static_cast<std::uint32_t>( i ) );
        std::memcpy( trec + 40, ExpertAccess::rawData( *segB ), 16 );//start time and energy
        trec += REC_TRACK;
        std::uint32_t iseg = 0;
        for ( auto seg = segB; seg != segE; ++seg ) {
          std::memcpy( srec, ExpertAccess::rawData( *seg ), RAW_SEGMENT );
          put32( srec + 48, static_cast<std::uint32_t>( i ) );
          put32( srec + 52, iseg++ );
          put32( srec + 56, static_cast<std::uint32_t>( seg - seg0 ) );
          put32( srec + 60, 0 );
          srec += REC_SEGMENT;
        }
      }
      t[TBL_NTRACKS] = static_cast<std::int64_t>( ntrk );
      t[TBL_TRACKS] = static_cast<std::int64_t>( trkoff );
      t[TBL_NSEGMENTS] = static_cast<std::int64_t>( nseg );
      t[TBL_SEGMENTS] = static_cast<std::int64_t>( segoff );
    }

    void actualDecodeSteps()
    {
      auto t = tbl();
      const std::size_t ntrk = static_cast<std::size_t>( t[TBL_NTRACKS] );
      const std::size_t nseg = static_cast<std::size_t>( t[TBL_NSEGMENTS] );
      if ( ntrk != m_dr->nTracks() )
        throw std::logic_error( "tracks must be decoded before steps" );
      const std::size_t segstepoff = ( t[TBL_SEGMENTS] + REC_SEGMENT * nseg + 7 ) & ~std::size_t(7);
      const std::size_t stepoff = segstepoff + 8 * nseg;
      char * ssrec = tblCheck( segstepoff, 8 * nseg );
      std::size_t nsteps = 0;
      const GriffDataRead::Track * trkBegin = m_dr->trackBegin();
      for ( std::size_t i = 0; i < ntrk; ++i ) {
        const auto& trk = trkBegin[i];
        std::uint32_t iseg = 0;
        for ( auto seg = trk.segmentBegin(); seg != trk.segmentEnd(); ++seg, ++iseg ) {
          std::int32_t rawstep;
          std::memcpy( &rawstep, ExpertAccess::rawData( *seg ) + 20, sizeof(rawstep) );
          const std::size_t first = nsteps;
          if ( rawstep >= 0 ) {
            std::uint32_t istep = 0;
            const auto stepE = seg->stepEnd();
            for ( auto step = seg->stepBegin(); step != stepE; ++step ) {
              char * rec = tblCheck( stepoff + REC_STEP * nsteps, REC_STEP );
              std::memcpy( rec, ExpertAccess::rawData( *step ), RAW_STEP );
              put32( rec + 152, static_cast<std::uint32_t>( i ) );
              put32( rec + 156, iseg );
              put32( rec + 160, istep++ );
              put32( rec + 164, 0 );
              ++nsteps;
            }
          }
          put32( ssrec, static_cast<std::uint32_t>( first ) );
          put32( ssrec + 4, static_cast<std::uint32_t>( nsteps - first ) );
          ssrec += 8;
        }
      }
      t[TBL_NSTEPS] = static_cast<std::int64_t>( nsteps );
      t[TBL_STEPS] = static_cast<std::int64_t>( stepoff );
      t[TBL_SEGSTEPS] = static_cast<std::int64_t>( segstepoff );
    }
    std::int64_t m_handle = 0;
    std::int64_t m_serial = 0;

    std::int64_t * hdr() { return reinterpret_cast<std::int64_t*>( m_buf.data() ); }
    void checkActive()
    {
      if ( !m_dr->eventActive() )
        throw std::runtime_error( "No active event" );
    }
    bool update( bool res )
    {
      updateHeader();
      return res;
    }
    void updateHeader()
    {
      auto fr = m_dr->getRawFileReader();
      if ( fr && fr->bad() )
        throw std::runtime_error( std::string( "GriffDataReader::ERROR Trouble while reading file " )
                                  + fr->fileName() + " : " + fr->bad_reason() );
      auto h = hdr();
      std::memset( h, 0, HDR_BYTES );
      h[HDR_SERIAL] = ++m_serial;
      h[HDR_LOOP_COUNT] = static_cast<std::int64_t>( m_dr->loopCount() );
      if ( !m_dr->eventActive() )
        return;
      h[HDR_ACTIVE] = 1;
      h[HDR_BRIEF_OFFSET] = HDR_BYTES;
      h[HDR_BRIEF_SIZE] = fr->nBytesBriefData();
      h[HDR_FULL_OFFSET] = static_cast<std::int64_t>( fr->fullDataOffset() );
      h[HDR_FULL_SIZE] = -1;//not loaded
      h[HDR_RUN] = fr->runNumber();
      h[HDR_EVENT] = fr->eventNumber();
      h[HDR_FILE_INDEX] = fileIndex();
      h[HDR_EVENT_INDEX] = m_dr->eventIndexInCurrentFile();
      h[HDR_CHECKSUM] = fr->eventCheckSum();
      h[HDR_VERSION] = fr->version();
      //The brief data is always needed:
      if ( !fr->getBriefData() )
        throw std::runtime_error( "Problems encountered while reading event data" );
    }
    std::int64_t fileIndex();
  };

}

std::int64_t GriffDataRead_py::Reader::fileIndex()
{
  return static_cast<std::int64_t>( ExpertAccess::fileIndex( *m_dr ) );
}

SBLDPY_MODULE( _impl )
{
  using GriffDataRead_py::Reader;
  using GriffDataRead::ExpertAccess;
  m.add_class<Reader>( "Reader" )
    .def_init<std::string,std::int64_t,sbldpy::NumpyView<std::uint8_t>,
              sbldpy::NumpyView<std::uint8_t>>( { "files", "nloops", "buffer", "tables" } )
    .def( "decode_tracks", &Reader::decodeTracks )
    .def( "decode_steps", &Reader::decodeSteps )
    .def( "handle", &Reader::handle )
    .def( "go_to_first_event", &Reader::goToFirstEvent )
    .def( "go_to_next_event", &Reader::goToNextEvent )
    .def( "go_to_next_file", &Reader::goToNextFile )
    .def( "skip_events", &Reader::skipEvents, { "n" } )
    .def( "loop_events", &Reader::loopEvents )
    .def( "seek_event", &Reader::seekEventByIndexInCurrentFile, { "idx" } )
    .def( "load_full_data", &Reader::loadFullData )
    .def( "verify_integrity", &Reader::verifyEventDataIntegrity )
    .def( "db_string", &Reader::dbString, { "subsect", "idx" } )
    .def( "volume_names", []( Reader& r, std::int64_t start ) { return ExpertAccess::volumeNames( r.dr(), start ); } )
    .def( "touchables", []( Reader& r, std::int64_t start ) { return ExpertAccess::touchables( r.dr(), start ); } )
    .def( "material", []( Reader& r, std::int64_t i ) { return ExpertAccess::material( r.dr(), i ); } )
    .def( "material_elements", []( Reader& r, std::int64_t i ) { return ExpertAccess::materialElements( r.dr(), i ); } )
    .def( "element", []( Reader& r, std::int64_t i ) { return ExpertAccess::element( r.dr(), i ); } )
    .def( "element_isotopes", []( Reader& r, std::int64_t i ) { return ExpertAccess::elementIsotopes( r.dr(), i ); } )
    .def( "isotope", []( Reader& r, std::int64_t i ) { return ExpertAccess::isotope( r.dr(), i ); } )
    .def( "particle_definition", []( Reader& r, std::int64_t pdg ) { return ExpertAccess::particleDefinition( r.dr(), pdg ); } )
    .def( "metadata", []( Reader& r, std::int64_t i ) { return ExpertAccess::metaData( r.dr(), i ); } );
  m.def( "set_open_msg", []( bool b ) { GriffDataReader::setOpenMsg( b ); } );
  m.def( "header_bytes", []() { return static_cast<std::int64_t>( GriffDataRead_py::HDR_BYTES ); } );
}
