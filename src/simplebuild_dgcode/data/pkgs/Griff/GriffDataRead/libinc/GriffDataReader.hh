#ifndef GriffDataRead_GriffDataReader_hh
#define GriffDataRead_GriffDataReader_hh

#include "Core/Types.hh"
#include "GriffDataRead/Touchable.hh"
#include "GriffDataRead/Material.hh"
#include "GriffDataRead/Element.hh"
#include "GriffDataRead/Isotope.hh"
#include "GriffDataRead/PDGCodeReader.hh"
#include "GriffDataRead/MetaData.hh"
#include "GriffDataRead/Setup.hh"
#include "GriffFormat/Format.hh"
#include "EvtFile/FileReader.hh"
#include "EvtFile/DBSubSectReaderMgr.hh"
#include "EvtFile/DBStringsReader.hh"
#include "EvtFile/DBEntryReader.hh"
#include "Utils/ByteStream.hh"
#include "Utils/MemPool.hh"
#include "Utils/DynBuffer.hh"
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>

class GriffDataReader;
namespace GriffDataRead {
  class Track;
  class Segment;
  class Step;
  struct BeginEventCallBack {
    virtual void beginEvent(const GriffDataReader*) = 0;
    virtual void dereg(const GriffDataReader*) {}
  };
  struct EndEventCallBack {
    virtual void endEvent(const GriffDataReader*) = 0;
    virtual void dereg(const GriffDataReader*) {}
  };
}

class GriffDataReader
{
public:

  //Constructor based on the name of one or several input files. Note that the names can contain wildcards.
  GriffDataReader(const std::string& inputFile, unsigned nloops = 1);
  GriffDataReader(const std::vector<std::string>& inputFiles, unsigned nloops = 1);

  //Special constructor for easily making griff analysis programs which reads
  //options from the command line in a standardised way:
  GriffDataReader(int argc,char**argv);


  ~GriffDataReader();

  //nloops=1 means run through all the files once, nloops>1 for more loops,
  //nloops=0 for infinite looping through input files.

  /////////////////////////
  //  Event data access  //
  /////////////////////////

  //eventActive() returns false if not at a readable event. This can occur as a
  //result of a failed navigation attempt or right away in case there are no
  //events in the input file(s)::
  bool eventActive() const;

  //Actual data access. Do not call before testing eventActive above.
  unsigned runNumber() const;
  unsigned eventNumber() const;
  int32_t currentEventVersion() const;
  GriffFormat::Format::MODE eventStorageMode() const;
  const char * eventStorageModeStr() const;
  std::uint64_t seed() const;//Random seed used for event generation.
  std::string seedStr() const;//as string for convenience

  bool setupChanged();//true if setup() gives different instance than at previous call
  GriffDataRead::Setup* setup();//Note that the returned instance is deleted at the end of the event unless you call ref() on it.
  void allowSetupChange() { m_allowSetupChange = true; }//if not called, reader will bail out upon encountering events with different settings.

  //access stored tracks in current event:
  unsigned nTracks() const;
  unsigned nPrimaryTracks() const;
  const GriffDataRead::Track* getTrack(unsigned i) const;

  //The next two methods might be inefficient when the event was filtered and
  //not all tracks are present:
  bool hasTrackID(int id) const;
  const GriffDataRead::Track* getTrackByID(int id) const;//undefined if no track with such id in the event

  //iterate over tracks in current event:
  const GriffDataRead::Track* trackBegin() const;
  const GriffDataRead::Track* trackEnd() const;
  const GriffDataRead::Track* primaryTrackBegin() const;
  const GriffDataRead::Track* primaryTrackEnd() const;

  //Note that if you want to go through all tracks, it is more efficient to use
  //trackBegin/End directly rather than starting with primaryTrackBegin/End and
  //going through the daughters.

  //Note that from python, you can iterate over tracks by the following examples
  //(assuming dr is a GriffDataReader object with dr.eventActive() true):
  //
  //for trk in datareader.tracks:
  //   print trk.trackID()
  //   ...
  //for trk in datareader.primaryTracks:
  //   print trk.trackID(),trk.pdgName()
  //   ...

  ////////////////////////
  //  Event navigation  //
  ////////////////////////

  //The following methods will all return false when the navigation fails:
  bool goToNextFile();//try to skip to first event in next file
  bool goToNextEvent();//will automatically skip to next file when needed
  bool goToFirstEvent();//meaning first event of the first file with n>0 events in it
  bool skipEvents(unsigned n);//Will automatically skip to next file when needed.

  //If all you want to do is loop through all events, it is simplest to use the
  //following method which combines eventActive() checks with goToNextEvent():

  bool loopEvents();//reset afterwards by goToFirstEvent()
  std::uint64_t loopCount() const;//loop counter (0=>first event, 1=>second event, ...)

  //callbacks (must live for longer than the GriffDataReader is being used to
  //navigate events, or be deregistered):
  void registerBeginEventCallBack(GriffDataRead::BeginEventCallBack*);
  void registerEndEventCallBack(GriffDataRead::EndEventCallBack*);
  void deregisterBeginEventCallBack(GriffDataRead::BeginEventCallBack*);
  void deregisterEndEventCallBack(GriffDataRead::EndEventCallBack*);

  //Special methods for jumping within the same file (users must be careful when their code might one day run on multiple files!):
  bool seekEventByIndexInCurrentFile(unsigned idx);
  unsigned eventIndexInCurrentFile() const;

  //Special methods for data hashing / integrity
  std::uint32_t eventCheckSum() const;//Checksum stored in file
  bool verifyEventDataIntegrity();//Recalculate checksum and verify

private:
  //multiple files:
  std::vector<std::string> m_inputFiles;
  unsigned m_loopsOrig;//should be const after constructor (for c++11?)
  unsigned m_loops;
  std::uint64_t m_loopCount;
  bool m_eventLoopStart;
  //current file:
  unsigned m_fileIdx;
  EvtFile::FileReader * m_fr;
  alignas(EvtFile::FileReader) char m_mempool_filereader[sizeof(EvtFile::FileReader)];
  //event data:
  mutable bool m_needsLoad;
  mutable bool m_tracksContiguous;
  //for the user-visible setup() and setupChanged():
  std::pair<EvtFile::index_type,unsigned> m_lastAccessedMetaDataIdx;//(metadataidx,fileIdx)
  GriffDataRead::Setup * m_setup;
  //for the behind-the-scenes monitoring of setup consistency:
  bool m_allowSetupChange;
  std::pair<EvtFile::index_type,unsigned> m_cachedMetaDataIdx;//(metadataidx,fileIdx)
  GriffDataRead::StrMap m_cachedMetaData;


  mutable const GriffDataRead::Track* m_tracksBegin;
  mutable const GriffDataRead::Track* m_tracksEnd;
  mutable const GriffDataRead::Track* m_primaryTracksBegin;
  mutable const GriffDataRead::Track* m_primaryTracksEnd;
  mutable std::vector<GriffDataRead::Track> m_tracks;
  //mutable std::vector<char> m_mempool_segments;//fixme align!
  mutable Utils::DynBuffer<char> m_mempool_segments;//fixme align!
  static const unsigned MEMPOOL_STEPS_NSTEPS = 10;
  static const unsigned STEP_SIZE = 2*sizeof(void*);//to avoid include loop
  Utils::MemPool<STEP_SIZE*MEMPOOL_STEPS_NSTEPS> m_mempool_steps;//fixme align
  std::vector<char*> m_mempool_dynamic;//stuff to delete[] in clearEvent

  //db:
  EvtFile::DBSubSectReaderMgr m_dbmgr;
  EvtFile::DBEntryReader<GriffDataRead::Touchable> m_dbTouchables;
  EvtFile::DBStringsReader m_dbVolNames;
  EvtFile::DBEntryReader<GriffDataRead::Material> m_dbMaterials;
  EvtFile::DBEntryReader<GriffDataRead::Element> m_dbElements;
  EvtFile::DBEntryReader<GriffDataRead::Isotope> m_dbIsotopes;
  EvtFile::DBStringsReader m_dbMaterialNames;
  EvtFile::DBStringsReader m_dbElementNames;
  EvtFile::DBStringsReader m_dbIsotopeNames;
  EvtFile::DBStringsReader m_dbProcNames;
  GriffDataRead::PDGCodeReader m_dbPDGCodes;
  EvtFile::DBStringsReader m_dbPDGNames;
  EvtFile::DBStringsReader m_dbPDGTypes;
  EvtFile::DBStringsReader m_dbPDGSubTypes;
  EvtFile::DBEntryReader<GriffDataRead::MetaData> m_dbMetaData;
  EvtFile::DBStringsReader m_dbMetaDataStrings;

  //callbacks:
  std::vector<GriffDataRead::BeginEventCallBack*> m_beginEventCallBacks;
  std::vector<GriffDataRead::EndEventCallBack*> m_endEventCallBacks;

  void init();
  void initFile(unsigned);
  void loadTracks() const;
  void actualLoadTracks() const;
  void clearEvent();
  void beginEventActions();
  void fireBeginEventCallBacks();
  bool setupChangedFullCheck(EvtFile::index_type current_mdidx);
  void checkSetupConsistency();
  EvtFile::index_type metaDataIdx() const;


  friend class GriffDataRead::Track;
  friend class GriffDataRead::Segment;
  friend class GriffDataRead::Step;
  friend class GriffDataRead::Material;
  friend class GriffDataRead::Element;
  friend class GriffDataRead::Isotope;
  friend class GriffDataRead::Touchable;
  friend class GriffDataRead::MetaData;

  static bool sm_openMsg;
public:
  //enable/disable the message printed when opening a file:
  static void setOpenMsg(bool b) { sm_openMsg = b; }
  static bool openMsg() { return sm_openMsg; }

  EvtFile::FileReader * getRawFileReader() { return m_fr; }//Experts only!

};

#ifndef GriffDataRead_Track_hh
#include "GriffDataRead/Track.hh"
#endif
#ifndef GriffDataRead_Segment_hh
#include "GriffDataRead/Segment.hh"
#endif
#ifndef GriffDataRead_Step_hh
#include "GriffDataRead/Step.hh"
#endif
#include "GriffDataReader.icc"
#endif
