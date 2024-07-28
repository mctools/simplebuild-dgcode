#include "GriffDataRead/GriffDataReader.hh"
#include "Utils/Glob.hh"
#include "Core/File.hh"
#include "Core/String.hh"
#include <sstream>
#include <stdexcept>

bool GriffDataReader::sm_openMsg = true;

GriffDataReader::GriffDataReader(const std::string& inputFile, unsigned nloops)
  : m_loopsOrig(nloops),m_loops(nloops),
    m_dbTouchables(GriffFormat::Format::subsectid_touchables),
    m_dbVolNames(GriffFormat::Format::subsectid_volnames),
    m_dbMaterials(GriffFormat::Format::subsectid_materials),
    m_dbElements(GriffFormat::Format::subsectid_elements),
    m_dbIsotopes(GriffFormat::Format::subsectid_isotopes),
    m_dbMaterialNames(GriffFormat::Format::subsectid_materialnames),
    m_dbElementNames(GriffFormat::Format::subsectid_elementnames),
    m_dbIsotopeNames(GriffFormat::Format::subsectid_isotopenames),
    m_dbProcNames(GriffFormat::Format::subsectid_procnames),
    m_dbPDGCodes(GriffFormat::Format::subsectid_pdgcodes),
    m_dbPDGNames(GriffFormat::Format::subsectid_pdgnames),
    m_dbPDGTypes(GriffFormat::Format::subsectid_pdgtypes),
    m_dbPDGSubTypes(GriffFormat::Format::subsectid_pdgsubtypes),
    m_dbMetaData(GriffFormat::Format::subsectid_metadata),
    m_dbMetaDataStrings(GriffFormat::Format::subsectid_metadatastrings)
{
  m_inputFiles.push_back(inputFile);
  init();
}

GriffDataReader::GriffDataReader(const std::vector<std::string>& inputFiles, unsigned nloops)
  : m_inputFiles(inputFiles),
    m_loopsOrig(nloops),m_loops(nloops),
    m_dbTouchables(GriffFormat::Format::subsectid_touchables),
    m_dbVolNames(GriffFormat::Format::subsectid_volnames),
    m_dbMaterials(GriffFormat::Format::subsectid_materials),
    m_dbElements(GriffFormat::Format::subsectid_elements),
    m_dbIsotopes(GriffFormat::Format::subsectid_isotopes),
    m_dbMaterialNames(GriffFormat::Format::subsectid_materialnames),
    m_dbElementNames(GriffFormat::Format::subsectid_elementnames),
    m_dbIsotopeNames(GriffFormat::Format::subsectid_isotopenames),
    m_dbProcNames(GriffFormat::Format::subsectid_procnames),
    m_dbPDGCodes(GriffFormat::Format::subsectid_pdgcodes),
    m_dbPDGNames(GriffFormat::Format::subsectid_pdgnames),
    m_dbPDGTypes(GriffFormat::Format::subsectid_pdgtypes),
    m_dbPDGSubTypes(GriffFormat::Format::subsectid_pdgsubtypes),
    m_dbMetaData(GriffFormat::Format::subsectid_metadata),
    m_dbMetaDataStrings(GriffFormat::Format::subsectid_metadatastrings)
{
  init();
}

GriffDataReader::GriffDataReader(int argc, char**argv)
  : m_dbTouchables(GriffFormat::Format::subsectid_touchables),
    m_dbVolNames(GriffFormat::Format::subsectid_volnames),
    m_dbMaterials(GriffFormat::Format::subsectid_materials),
    m_dbElements(GriffFormat::Format::subsectid_elements),
    m_dbIsotopes(GriffFormat::Format::subsectid_isotopes),
    m_dbMaterialNames(GriffFormat::Format::subsectid_materialnames),
    m_dbElementNames(GriffFormat::Format::subsectid_elementnames),
    m_dbIsotopeNames(GriffFormat::Format::subsectid_isotopenames),
    m_dbProcNames(GriffFormat::Format::subsectid_procnames),
    m_dbPDGCodes(GriffFormat::Format::subsectid_pdgcodes),
    m_dbPDGNames(GriffFormat::Format::subsectid_pdgnames),
    m_dbPDGTypes(GriffFormat::Format::subsectid_pdgtypes),
    m_dbPDGSubTypes(GriffFormat::Format::subsectid_pdgsubtypes),
    m_dbMetaData(GriffFormat::Format::subsectid_metadata),
    m_dbMetaDataStrings(GriffFormat::Format::subsectid_metadatastrings)
{
  //Todo: decode -h, --helps, --loops=L, --maxevts=M
  std::string tmp;
  for (int i=1;i<argc;++i) {
    tmp=argv[i];
    if (!tmp.empty() && tmp[0]!='-' ) {
      m_inputFiles.push_back(tmp);
    }
  }
  const unsigned nloops = 1;
  m_loopsOrig = m_loops = nloops;
  init();
}

GriffDataReader::~GriffDataReader()
{
  for (auto it = m_beginEventCallBacks.begin();it!=m_beginEventCallBacks.end();++it)
    (*it)->dereg(this);
  m_beginEventCallBacks.clear();
  for (auto it = m_endEventCallBacks.begin();it!=m_endEventCallBacks.end();++it)
    (*it)->dereg(this);
  m_endEventCallBacks.clear();
  if (m_setup)
    m_setup->unref();
  static_assert(GriffDataReader::STEP_SIZE==sizeof(GriffDataRead::Step));
  if (m_fr)
    m_fr->EvtFile::FileReader::~FileReader();
}

void GriffDataReader::deregisterBeginEventCallBack(GriffDataRead::BeginEventCallBack*cb)
{
  cb->dereg(this);
  auto it = std::find(m_beginEventCallBacks.begin(), m_beginEventCallBacks.end(), cb);
  if (it == m_beginEventCallBacks.end())
    throw std::runtime_error("inconsistent call to deregisterBeginEventCallBack");
  m_beginEventCallBacks.erase(it);
}

void GriffDataReader::deregisterEndEventCallBack(GriffDataRead::EndEventCallBack*cb) {
  cb->dereg(this);
  auto it = std::find(m_endEventCallBacks.begin(), m_endEventCallBacks.end(), cb);
  if (it == m_endEventCallBacks.end())
    throw std::runtime_error("inconsistent call to deregisterEndEventCallBack");
  m_endEventCallBacks.erase(it);
}

void GriffDataReader::init()
{
  //Expand via glob any '*' chars in inputfiles
  unsigned nPatterns(m_inputFiles.size());
  std::vector<std::string> tmp_infiles;
  tmp_infiles.reserve(m_inputFiles.size());
  auto itFE=m_inputFiles.end();
  for (auto itF = m_inputFiles.begin(); itF!=itFE; ++itF) {
    if (itF->find('*')!=std::string::npos) {
      Utils::glob(*itF,tmp_infiles);
    } else {
      tmp_infiles.push_back(*itF);
    }
  }
  m_inputFiles = tmp_infiles;

  //Check that all input files exists (better to die immediately than to crunch
  //through a few files and then die when a non-existing file is reached):
  if (m_inputFiles.empty()) {
    if (nPatterns)
      printf("\nGriffDataReader::ERROR No files found based on specified input file patterns!\n\n");
    else
      printf("\nGriffDataReader::ERROR No input files specified!\n\n");
    assert(false&&"No input files specifed");
    exit(1);
  } else {
    itFE=m_inputFiles.end();
    for (auto itF = m_inputFiles.begin(); itF!=itFE; ++itF) {
      if (!Core::file_exists(*itF)) {
        printf("GriffDataReader::ERROR Input file does not exist: %s\n",itF->c_str());
        assert(false&&"Input file does not exist");
        exit(1);
      }
    }
  }
  m_loopCount = 0;
  m_setup = 0;
  m_allowSetupChange = false;//by default we are conservative
  m_lastAccessedMetaDataIdx.first = EvtFile::INDEX_MAX;
  m_lastAccessedMetaDataIdx.second = UINT_MAX;
  m_cachedMetaDataIdx = m_lastAccessedMetaDataIdx;

  m_tracksContiguous = false;

  m_tracksBegin = 0;
  m_tracksEnd = 0;
  m_primaryTracksBegin = 0;
  m_primaryTracksEnd = 0;

  m_dbmgr.addSubSection(m_dbTouchables);
  m_dbmgr.addSubSection(m_dbVolNames);
  m_dbmgr.addSubSection(m_dbMaterials);
  m_dbmgr.addSubSection(m_dbElements);
  m_dbmgr.addSubSection(m_dbIsotopes);
  m_dbmgr.addSubSection(m_dbMaterialNames);
  m_dbmgr.addSubSection(m_dbElementNames);
  m_dbmgr.addSubSection(m_dbIsotopeNames);
  m_dbmgr.addSubSection(m_dbProcNames);
  m_dbmgr.addSubSection(m_dbPDGCodes);
  m_dbmgr.addSubSection(m_dbPDGNames);
  m_dbmgr.addSubSection(m_dbPDGTypes);
  m_dbmgr.addSubSection(m_dbPDGSubTypes);
  m_dbmgr.addSubSection(m_dbMetaData);
  m_dbmgr.addSubSection(m_dbMetaDataStrings);

  m_fileIdx = UINT_MAX;
  m_fr = 0;
  goToFirstEvent();
}

void GriffDataReader::initFile(unsigned i)
{
  clearEvent();
  assert(i<m_inputFiles.size());
  if (m_fr) {
    if (m_fr->ok()&&m_inputFiles[i]==m_fr->fileName()) {
      //special case: if it is the same file as the current, we just jump back to the first event:
#ifdef NDEBUG
      m_fr->goToFirstEvent();
#else
      bool ok = m_fr->goToFirstEvent();
      assert(m_fr->nBytesBriefData()>=2*sizeof(std::uint32_t));
      assert(ok&&m_fr->ok()&&!m_fr->bad());
      assert(m_fr->verifyEventDataIntegrity());
#endif
      m_fileIdx = i;
      return;
    }
    m_fr->EvtFile::FileReader::~FileReader();//fixme std::optional would be better!
    m_fr = nullptr;
  }
  m_fr = new(&(m_mempool_filereader[0])) EvtFile::FileReader(GriffFormat::Format::getFormat(),m_inputFiles[i].c_str(),&m_dbmgr);
  bool ok = m_fr->init();
  if (!ok || m_fr->bad()) {
    printf("GriffDataReader::ERROR Trouble while opening file %s : %s\n",m_inputFiles[i].c_str(),m_fr->bad_reason());
    assert(false&&"could not open file");
  } else {
    if (openMsg()) {
      std::vector<std::string> parts;
      Core::split_noempty(parts,m_fr->fileName(),"/");
      printf("GriffDataReader opened file %s\n",parts.empty()? "?" : parts.back().c_str());
    }
  }
  m_fileIdx = i;
}

bool GriffDataReader::goToNextFile()
{
  if (m_fileIdx==m_inputFiles.size())
    return false;//we already determined that there is no next file to try.
  clearEvent();

  //Try next file:
  ++m_fileIdx;
  if (m_fileIdx==m_inputFiles.size()) {
    //We were already in the last file. Do we loop?
    if (m_loops==0) {
      //yep, loop forever
      m_fileIdx = 0;
    } else if (m_loops>1) {
      //one more loop
      --m_loops;
      m_fileIdx = 0;
    }
  }
  if (m_fileIdx==m_inputFiles.size())
    return false;//there is no next file to try
  initFile(m_fileIdx);
  if (eventActive())
    beginEventActions();
  return eventActive();
}

void GriffDataReader::actualLoadTracks() const
{
  assert(m_needsLoad);
  m_needsLoad=false;

  m_tracksContiguous = true;

  const unsigned ntracks = nTracks();

  if (!ntracks) {
    m_tracksEnd = m_tracksBegin = 0;
    m_primaryTracksEnd = m_primaryTracksBegin = 0;
    return;
  }
  //Grow track containers if needed:
  if (m_tracks.capacity()<ntracks) {
    unsigned nreserve(512);
    while(nreserve<ntracks)
      nreserve*=2;
    m_tracks.reserve(nreserve);
  }
  m_tracksBegin = &(*m_tracks.begin());
  m_tracksEnd = m_tracksBegin + ntracks;
  assert(m_tracksBegin);

  const char * data = m_fr->getBriefData();
#ifndef NDEBUG
  const char * dataE = data + m_fr->nBytesBriefData();
#endif
  data += GriffFormat::Format::SIZE_TRACKHEADER;

  unsigned iposp1(1);
  unsigned nsegments(0);

  //Create all tracks - and check whether they are contiguous, i.e. no holes in trkID.
  for (auto trk=m_tracksBegin;trk!=m_tracksEnd;++trk) {
    const_cast<GriffDataRead::Track*>(trk)->set(const_cast<GriffDataReader*>(this),data);
    data += GriffFormat::Format::SIZE_PER_TRACK_WO_DAUGHTERLIST + trk->nDaughters() * GriffFormat::Format::SIZE_PER_DAUGHTERLIST_ENTRY;
    if (trk->trackID()!=(int)(iposp1++))
      m_tracksContiguous = false;//we can't easily look up tracks by track ID later.
    nsegments+=trk->nSegments();
  }

  //Setup primary track pointers:
  if (m_tracksBegin->isPrimary()) {
    m_primaryTracksBegin = m_tracksBegin;
    for (auto trk=m_tracksBegin+1;trk!=m_tracksEnd;++trk) {
      if (!trk->isPrimary()) {
        m_primaryTracksEnd = trk;
        break;
      }
    }
#ifndef NDEBUG
    //sanity check - remaining tracks are secondaries
    if (m_primaryTracksEnd) {
      for (auto trk=m_primaryTracksEnd+1;trk!=m_tracksEnd;++trk) {
        assert(!trk->isPrimary());
      }
    }
#endif
    if (!m_primaryTracksEnd)
      m_primaryTracksEnd = m_tracksEnd;//only primary tracks in file
  } else {
    //no primary tracks saved
    m_primaryTracksBegin = m_primaryTracksEnd = 0;
  }

  assert(m_primaryTracksEnd>=m_primaryTracksBegin);

  //Segments:
  //  m_mempool_segments.reserve(nsegments*sizeof(GriffDataRead::Segment));
  m_mempool_segments.resize_without_init(nsegments*sizeof(GriffDataRead::Segment));
  unsigned iseg(0);
  for (GriffDataRead::Track* it=const_cast<GriffDataRead::Track*>(m_tracksBegin);it!=const_cast<GriffDataRead::Track*>(m_tracksEnd);++it) {

    //first segment on the track:
    it->m_firstSegment = new(&(m_mempool_segments[(iseg++)*sizeof(GriffDataRead::Segment)])) GriffDataRead::Segment(&(*it),data);
    data+=GriffFormat::Format::SIZE_PER_SEGMENT;
    if (it->m_firstSegment->nextWasFiltered()) {
      data+=GriffFormat::Format::SIZE_LAST_SEGMENT_ON_TRACK_EXTRA_SIZE;//extra info since filtering took place
    }
    //remaining segments on the track:
    unsigned n(it->nSegments());
    assert(n>=1);
    for (unsigned i=1;i<n;++i) {
      auto theseg = new(&(m_mempool_segments[(iseg++)*sizeof(GriffDataRead::Segment)])) GriffDataRead::Segment(&(*it),data);
      data+=GriffFormat::Format::SIZE_PER_SEGMENT;
      if (theseg->nextWasFiltered()) {
        data+=GriffFormat::Format::SIZE_LAST_SEGMENT_ON_TRACK_EXTRA_SIZE;//extra info since filtering took place
      }
    }
    data+=GriffFormat::Format::SIZE_LAST_SEGMENT_ON_TRACK_EXTRA_SIZE;//extra info for last segment
  }
  assert(data==dataE);
}

bool GriffDataReader::setupChangedFullCheck(EvtFile::index_type current_mdidx)
{
  assert(eventActive());
  if (m_lastAccessedMetaDataIdx.first==EvtFile::INDEX_MAX)
    return true;

  assert(m_setup);//should only get here if setup was accessed previously

  if (!m_allowSetupChange)
    return false;//if setup had changed we would have exited

  //File changed. It means that we need to create and compare the whole new map
  //and compare. Fortunately it only happens once per file.
  if (m_dbMetaData.getEntry(current_mdidx).getMap(this) != m_setup->allData()) {
    //Make sure we do not have to repeat the expensive check:
    m_lastAccessedMetaDataIdx.first = EvtFile::INDEX_MAX;
    m_lastAccessedMetaDataIdx.second = EvtFile::INDEX_MAX;
    return true;
  } else {
    //The same => update the last accessed indices to work for the current file:
    m_lastAccessedMetaDataIdx.first = current_mdidx;
    m_lastAccessedMetaDataIdx.second = m_fileIdx;
    return false;
  }
}

GriffDataRead::Setup* GriffDataReader::setup()
{
  if (!setupChanged())
    return m_setup;

  if (m_setup) {
    m_setup->unref();
    m_setup = 0;
  }
  EvtFile::index_type mdidx = metaDataIdx();
  m_setup = new GriffDataRead::Setup(m_dbMetaData.getEntry(mdidx).getMap(this));
  m_setup->ref();
  m_lastAccessedMetaDataIdx.first = mdidx;
  m_lastAccessedMetaDataIdx.second = m_fileIdx;
  return m_setup;
}

std::string GriffDataReader::seedStr() const
{
  std::ostringstream s;
  s << seed();
  return s.str();
}

void GriffDataReader::checkSetupConsistency()
{
  //Whenever fileidx/mdidx was different than in previous case, carry out a
  //check. The trick is that it should not change the experience of
  //setupChange() for the user so we use different variables for this!!

  EvtFile::index_type mdidx = metaDataIdx();
  if (m_cachedMetaDataIdx.first==EvtFile::INDEX_MAX) {
    //first time, simply cache:
    m_cachedMetaDataIdx = std::make_pair(mdidx,m_fileIdx);
    m_cachedMetaData = m_dbMetaData.getEntry(mdidx).getMap(this);
  } else {
    //test for consistency
    bool ok(true);
    if (m_fileIdx!=m_cachedMetaDataIdx.second) {
      //new file
      if (m_cachedMetaData!=m_dbMetaData.getEntry(mdidx).getMap(this)) {
        ok=false;
      } else {
        m_cachedMetaDataIdx = std::make_pair(mdidx,m_fileIdx);
        //no need to update the m_cachedMetaData since we will die shortly
      }
    } else {
      if (mdidx!=m_cachedMetaDataIdx.first)
        ok=false;
    }
    if (!ok) {
      printf("\n");
      printf("GriffDataReader ERROR: Two events in the input file(s) were simulated using different setups.\n");
      printf("                       The job is aborted as a safeguard against inconsistent analysis results.\n");
      printf("                       If you actually wish to analyse events from multiple setups, then please\n");
      printf("                       call myDataReader.allowSetupChange() before looping over events.\n");
      printf("\n");
      exit(1);
    }
  }
}
