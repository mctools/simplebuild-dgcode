#include "EvtFile/FileWriter.hh"
#include "EvtFile/FileReader.hh"
#include "EvtFile/FileWriter.hh"
#include "EvtFileDefs.hh"
#include "Utils/ProgressiveHash.hh"
#include "ZLibUtils/Compress.hh"
#include <limits>
#include <cassert>

namespace EvtFile {

  FileReader::FileReader( const IFormat* format,
                          const char* filename,
                          EvtFileDB* db_listener,
                          int buffer_len )
    : m_format(format),
      m_isInitialised(false),
      m_buf(0),
      m_bufferLength(buffer_len),
      m_version(-1),
      m_bad(false),
      m_db_listener(db_listener),
      m_fulldata_size(0),
      m_fulldata_compressed(format->compressFullData()),
      m_briefdata_isloaded(false),
      m_fulldata_isloaded(false),
      m_currentEventInfo(nullptr),
      m_fileName(filename)
  {
    //NB: Rest of initialisation done in init()
  }

  bool FileReader::init()
  {
    assert(!isInit()&&"ERROR: FileReader::init() called twice!");
    if (isInit()) {
      m_bad = true;
      m_reason = "FileReader::init() called twice";
      return false;
    }

    m_isInitialised=true;

    m_buf = m_bufferLength ? new char[m_bufferLength] : 0;
    m_is.rdbuf()->pubsetbuf(m_buf, m_bufferLength );
    m_is.open(m_fileName.c_str(), std::ios::in | std::ios::binary);

    if (m_is.fail()||!m_is.is_open()) {
      m_bad=true;
      m_reason="Could not open file";
      close();
      return false;
    }
    std::uint32_t magic;
    read(magic);
    m_is.peek();//always peek before checking eof!
    if (m_is.fail()||m_is.eof()||magic!=m_format->magicWord()) {
      m_bad=true;
      m_reason="File not in right format";
      close();
      return false;
    }
    int32_t fileversion;
    static_assert(EVTFILE_FILE_HEADER_BYTES==sizeof(magic)+sizeof(fileversion));//make sure we are consistent with EvtFileDefs.hh
    read(fileversion);
    if (m_is.fail()||fileversion<0) {
      m_bad=true;
      m_reason="File not in right format";
      close();
      return false;
    }
    if (fileversion<2) {
      m_bad=true;
      m_reason="Version of file is too old and no longer supported";
      close();
      return false;
    }
    if (fileversion>EVTFILE_VERSION) {
      m_bad=true;
      m_reason="File format version is too new";
      close();
      return false;
    }
    //All ok!
    m_version=fileversion;
    m_section_briefdata.reserve(4096);
    m_section_fulldata.reserve(4096);
    m_section_fulldata_compressed.reserve(4096);
    m_evts.reserve(1000);
    initEventAtIndex(0);
    return ok();
  }

  FileReader::~FileReader()
  {
    if (m_db_listener)
      m_db_listener->clearInfo();
    if (is_open())
      close();
  }

  void FileReader::close()
  {
    m_is.close();
    delete[] m_buf;
    m_buf = 0;
  }

  bool FileReader::skipEvents(int n)
  {
    assert(isInit());

    if (m_bad)
      return false;

    if (!m_currentEventInfo)
      return false;

    if (n==0)
      return true;

    unsigned idx=m_currentEventInfo->evtIndex;

    if (n<0&&-n>static_cast<int>(idx))
      return false;//can't go back that many events!

    unsigned targetidx=idx+n;
    if (targetidx<m_evts.size()) {
      initEventAtIndex(targetidx);//Already read it, jump directly there
      return m_currentEventInfo!=nullptr;
    }

    //Ok, targetidx lies beyond the events already read. Jump to the first
    //unread event and step forward from there.
    initEventAtIndex(m_evts.size());
    while(m_currentEventInfo && targetidx>m_currentEventInfo->evtIndex)
      initEventAtIndex(m_evts.size());

    return m_currentEventInfo!=nullptr;
  }

  bool FileReader::seekEventByIndex(unsigned idx)
  {
    assert(isInit());
    if (m_bad||m_evts.empty())//m_evts.empty() means file has no events
      return false;

    if (m_currentEventInfo&&m_currentEventInfo->evtIndex==idx)
      return true;//we are already here

    if (idx<m_evts.size()) {
      clearEOF();
      m_currentEventInfo=&(m_evts[idx]);
      m_briefdata_isloaded = false;
      m_fulldata_isloaded = false;
      return true;//we previously read the event so can jump right to it
    }

    //We are left with no current event and the target index must be in some unread event if anywhere:
    if (!seekEventByIndex(m_evts.size()-1))
      return false;
    while (goToNextEvent()) {
      if (m_currentEventInfo&&m_currentEventInfo->evtIndex==idx)
        return true;
    }
    return false;
  }

  void FileReader::initEventAtIndex(unsigned idx)
  {
    assert(isInit());

    //This internal method can be used to either init a previously read event or
    //to init the first uninitialised event. It can not be used to spool
    //forward as it assumes the event is always there.
    if (m_currentEventInfo && m_currentEventInfo->evtIndex==idx)
      return;//already there

    m_currentEventInfo=nullptr;
    m_briefdata_isloaded = false;
    m_fulldata_isloaded = false;

    assert(idx<=m_evts.size());
    if (idx==m_evts.size()) {
      //Read the next event!
      assert(!m_bad);
      std::streampos newEvtPos;
      if (idx!=0) {
        //Seek to end of last read event in file:
        EventInfo& lastEvt = m_evts.back();
        newEvtPos = lastEvt.evtPosInFile;
        newEvtPos +=EVTFILE_EVENT_HEADER_BYTES;
        static_assert(EVTFILE_EVENT_HEADER_BYTES==6*sizeof(std::uint32_t));
        newEvtPos+=lastEvt.sectionSize_database;
        newEvtPos+=lastEvt.sectionSize_briefdata;
        newEvtPos+=lastEvt.sectionSize_fulldata;
      } else {
        //We got called from our own constructor so we are already here
        newEvtPos =m_is.tellg();
      }
      clearEOF();
      m_is.seekg(newEvtPos);
      m_is.peek();//always peek before checking eof!
      if (m_is.eof()) {
        //This is not an error condition - we simply reached the end of the file.
        return;
      }
      if (m_is.fail()) {
        m_bad=true;
        m_reason="Error while seeking to next event";
        return;
      }
      if (m_evts.capacity()==m_evts.size())
        m_evts.reserve(m_evts.size()*2);
      m_evts.emplace_back();//.resize(m_evts.size()+1);
      EventInfo& newEvt = m_evts.back();
      newEvt.evtPosInFile = newEvtPos;
      newEvt.evtIndex = m_evts.size()-1;
      read(reinterpret_cast<char*>(&newEvt.checkSum),sizeof(std::uint32_t)*6);//Trick to read all 6 variables with one call
      static_assert(sizeof(EventInfo)==sizeof(std::uint32_t)*8+sizeof(std::streampos));//make sure there is no padding => our trick would fail
      static_assert(EVTFILE_EVENT_HEADER_BYTES==sizeof(std::uint32_t)*6);//make sure we are consistent with EvtFileDefs.hh
      if (m_is.fail()) {
        m_bad=true;
        m_evts.resize(m_evts.size()-1);
        m_reason="Errors encountered while reading event header";
        return;
      }
      m_currentEventInfo=&newEvt;
      if (m_db_listener && newEvt.sectionSize_database) {
        //Read the database info and pass it on to any derived class.
        //For economical reasons we temporarily use the m_section_briefdata for this.
        assert(!m_briefdata_isloaded);
        m_section_briefdata.resize_without_init(newEvt.sectionSize_database);
        read(m_section_briefdata.data(),newEvt.sectionSize_database);
        if (m_is.fail()) {
          m_bad=true;
          m_evts.resize(m_evts.size()-1);
          m_reason="Errors encountered while reading database section of event";
          return;
        }
        m_db_listener->newInfoAvailable(m_section_briefdata.data(),newEvt.sectionSize_database);
      }
      //All ok it seems:
      m_currentEventInfo=&newEvt;
    } else {
      //Event has been read previously:
      m_currentEventInfo=&(m_evts[idx]);
    }
  }

  bool FileReader::goToEvent(std::uint32_t run_number,std::uint32_t evt_number)
  {
    assert(isInit());

    if (m_currentEventInfo&&m_currentEventInfo->evtNumber==evt_number&&m_currentEventInfo->runNumber==run_number)
      return true;//already there.

    m_currentEventInfo=nullptr;

    m_briefdata_isloaded = false;
    m_fulldata_isloaded = false;

    if (m_evtMap.size()<m_evts.size()) {
      //update m_evtMap
      for (unsigned i=m_evtMap.size();i<m_evts.size();++i) {
        m_evtMap[std::make_pair(m_evts[i].runNumber,m_evts[i].evtNumber)]=&m_evts[i];
      }
      assert(m_evtMap.size()==m_evts.size() && "Error: (runNbr,evtNbr) was not unique in file!");
    }
    auto it = m_evtMap.find(std::make_pair(run_number,evt_number));
    if (it!=m_evtMap.end()) {
      m_currentEventInfo=it->second;
      return true;
    }
    return false;
  }
  void FileReader::getSharedDataInEvent(std::vector<char>& data) {
    assert(isInit());
    assert(eventActive() && "getSharedDataInEvent() called when not eventActive()");

    assert(m_currentEventInfo!=nullptr);
    data.resize(m_currentEventInfo->sectionSize_database);
    if (!m_currentEventInfo->sectionSize_database)
      return;

    m_is.seekg(m_currentEventInfo->evtPosInFile+std::streampos(EVTFILE_EVENT_HEADER_BYTES));
    assert(m_is.tellg()==m_currentEventInfo->evtPosInFile+std::streampos(EVTFILE_EVENT_HEADER_BYTES));
    char * tmp = data.data();
    assert(tmp);
    read(tmp,m_currentEventInfo->sectionSize_database);
    return;
  }

  const char* FileReader::getBriefData() {
    assert(isInit());
    if (m_briefdata_isloaded)
      return m_section_briefdata.data();
    assert(eventActive() && "getBriefData() called when not eventActive()");
    unsigned n(nBytesBriefData());
    if (n) {
      m_section_briefdata.resize_without_init(nBytesBriefData());
      assert(m_currentEventInfo!=nullptr);
      std::streampos pos = m_currentEventInfo->evtPosInFile;
      pos+=EVTFILE_EVENT_HEADER_BYTES;
      pos+=m_currentEventInfo->sectionSize_database;
      m_is.seekg(pos);
      if (m_is.fail()) {
        m_bad=true;
        return 0;
      }
      read(m_section_briefdata.data(),n);
      if (m_is.fail()) {
        m_bad=true;
        return 0;
      }
    }
    m_briefdata_isloaded=true;
    return m_section_briefdata.data();
  }

  const char* FileReader::getFullData() {
    assert(isInit());

    if (m_fulldata_isloaded)
      return m_section_fulldata.data();

    assert(eventActive() && "getFullData() called when not eventActive()");

    unsigned n(nBytesFullDataOnDisk());
    m_fulldata_size = n;
    if (n) {
      //read compressed data:
      if (m_fulldata_compressed)
        m_section_fulldata_compressed.resize_without_init(n);
      else
        m_section_fulldata.resize_without_init(n);
      assert(m_currentEventInfo!=nullptr);
      std::streampos pos = m_currentEventInfo->evtPosInFile;
      pos+=EVTFILE_EVENT_HEADER_BYTES;
      pos+=m_currentEventInfo->sectionSize_database;
      pos+=m_currentEventInfo->sectionSize_briefdata;
      m_is.seekg(pos);
      if (m_is.fail()) {
        m_bad=true;
        return 0;
      }
      if (m_fulldata_compressed)
        read(m_section_fulldata_compressed.data(),n);
      else
        read(m_section_fulldata.data(),n);
      if (m_is.fail()) {
        m_bad=true;
        return 0;
      }
      if (m_fulldata_compressed) {
        //uncompress:
        assert(n>sizeof(std::uint32_t));
        ZLibUtils::decompressToBufferNew(m_section_fulldata_compressed.data(), n, m_section_fulldata);
        if ( m_section_fulldata.size() >= std::numeric_limits<unsigned>::max() )
          throw std::runtime_error("full data section size exceeds unsigned integer limits");
        m_fulldata_size = static_cast<unsigned>(m_section_fulldata.size());
      }
    }
    m_fulldata_isloaded=true;
    return m_section_fulldata.data();
  }

  bool FileReader::verifyEventDataIntegrity()
  {
    assert(isInit());

    if (!ok()||!eventActive())
      return false;
    assert(m_currentEventInfo);

    ProgressiveHash hash;
    hash.addData(reinterpret_cast<char*>(&(m_currentEventInfo->runNumber)),5*sizeof(std::uint32_t));

    if (m_currentEventInfo->sectionSize_database>0) {
      m_is.seekg(m_currentEventInfo->evtPosInFile+std::streampos(EVTFILE_EVENT_HEADER_BYTES));
      assert(m_is.tellg()==m_currentEventInfo->evtPosInFile+std::streampos(EVTFILE_EVENT_HEADER_BYTES));
      char * tmp = new char[m_currentEventInfo->sectionSize_database];//we could cache this, but normally we don't verify integrity...
      assert(tmp);
      read(tmp,m_currentEventInfo->sectionSize_database);
      hash.addData(tmp,m_currentEventInfo->sectionSize_database);
      delete[] tmp;
    }

    if (m_currentEventInfo->sectionSize_briefdata)
      hash.addData(getBriefData(),m_currentEventInfo->sectionSize_briefdata);

    if (m_currentEventInfo->sectionSize_fulldata) {
      hash.addData(getFullData(),nBytesFullData());
    }

    return m_currentEventInfo->checkSum == hash.getHash();
  }

}
