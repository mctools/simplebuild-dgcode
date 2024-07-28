#ifndef EvtFile_FileReader_hh
#define EvtFile_FileReader_hh

#ifndef __STDC_LIMIT_MACROS
#  define __STDC_LIMIT_MACROS
#endif

//Class responsible for actual reading of the data files.

#include "Core/Types.hh"
#include "EvtFile/IFormat.hh"
#include "EvtFile/IDBSubSectionReader.hh"
#include <vector>
#include <fstream>
#include <cstring>
#include <map>
#include <cassert>
#include "Utils/DynBuffer.hh"

namespace EvtFile {

  class EvtFileDB {
    //Derived classes can be registered with a FileReader in the constructor
  public:
    virtual void newInfoAvailable(const char*data, unsigned nbytes) = 0;
    virtual void clearInfo() = 0;//must clear all info upon request (normally when opening a new file)
    EvtFileDB(){}
    virtual ~EvtFileDB(){}
  };

  class FileReader final {
  public:

    ////////////////////////////////////////////
    //  Methods for opening/closing the file  //
    ////////////////////////////////////////////

    //The constructor opens the file and writes the first few words identifying
    //the filetype.
    FileReader( const IFormat*,
                const char* filename,
                EvtFileDB* db_listener = 0,
                int buffer_len=8192 );

    FileReader( const FileReader& ) = delete;
    FileReader& operator=( const FileReader& ) = delete;
    FileReader( FileReader&& ) = delete;
    FileReader& operator=( FileReader&& ) = delete;

    bool isInit() const;//returns true after init() has been called.

    bool init();//Actually opens file and seeks to the first event if
                //any. Returns true if all ok. NB: Even returns true on a file
                //with zero events as there can be valid use-cases for files
                //with zero events. So remember to check eventActive() to see if
                //first event was loaded.

    //File should be open after init was run unless an error
    //occured. It will also cease to be considered open after a call to close():
    bool is_open() const { return m_is.is_open(); }

    bool bad() const { return m_bad; }
    const char * bad_reason() { return m_reason.c_str(); }

    bool ok() const { return is_open() && !bad(); }

    //filename:
    const char * fileName() const { return m_fileName.c_str(); }

    //Call to close the file:
    void close();

    //The destructor will call close() if the file is still open:
    ~FileReader();

    int32_t version() const { return m_version; }

    ////////////////////////
    //  Event navigation  //
    ////////////////////////

    //The following methods will all return false when the navigation fails
    bool goToNextEvent() { return skipEvents(1); }
    bool goToPreviousEvent() { return skipEvents(-1); }
    bool goToFirstEvent() { return seekEventByIndex(0); };
    bool skipEvents(int n);//n<0 to go backwards
    bool seekEventByIndex(unsigned idx);//event idx in the file [0=first evt in file, 1=second, etc.]
    bool goToEvent(std::uint32_t run_number,std::uint32_t evt_number);

    /////////////////////////
    //  Event data access  //
    /////////////////////////

    //eventActive() returns false if not at a readable event. This can occur as
    //a result of a failed navigation attempt or right after the constructor in
    //case the file has 0 events:
    bool eventActive() const { return m_currentEventInfo!=nullptr; }

    //Actual data access. Do not call before testing eventActive above.
    unsigned runNumber() const { return m_currentEventInfo->runNumber; }
    unsigned eventNumber() const { return m_currentEventInfo->evtNumber; }
    unsigned eventIndex() const { return m_currentEventInfo->evtIndex; }
    unsigned nBytesBriefData() const { return m_currentEventInfo->sectionSize_briefdata; }
    unsigned nBytesFullData() { if (!m_fulldata_isloaded) getFullData(); return m_fulldata_size; }
    unsigned nBytesFullDataOnDisk() const { return m_currentEventInfo->sectionSize_fulldata; }
    const char* getBriefData();//on demand loading => not const (we could consider mutable, but...)
    const char* getFullData();//on demand loading => not const (we could consider mutable, but...)

    //Special methods for data hashing / integrity
    std::uint32_t eventCheckSum() const;//Checksum stored in file
    bool verifyEventDataIntegrity();//Recalculate checksum and verify
                                    //(Semi-expensive, will re-read the DB
                                    //section each time called)

    //Special methods for specialised low-level access:
    unsigned nBytesSharedDataInEvent() const { return m_currentEventInfo->sectionSize_database; }
    void getSharedDataInEvent(std::vector<char>&);//Will reload data and place in vector (will be resized)

  private:


    // FileReader( const FileReader & );
    // FileReader & operator= ( const FileReader & );

    const IFormat* m_format;
    bool m_isInitialised;
    char * m_buf;
    unsigned m_bufferLength;
    std::ifstream m_is;
    int32_t m_version;
    //bool m_eventActive;
    bool m_bad;
    EvtFileDB * m_db_listener;
    std::string m_reason;

    void read(char*data,unsigned nbytes);
    template<class T>
    void read(T&t);
    void clearEOF();

    Utils::DynBuffer<char> m_section_briefdata;
    Utils::DynBuffer<char> m_section_fulldata;
    unsigned m_fulldata_size;
    bool m_fulldata_compressed;
    Utils::DynBuffer<char> m_section_fulldata_compressed;
    bool m_briefdata_isloaded;
    bool m_fulldata_isloaded;

    void initEventAtIndex(unsigned idx);
    struct EventInfo {
      std::uint32_t checkSum;
      std::uint32_t runNumber;
      std::uint32_t evtNumber;
      std::uint32_t sectionSize_database;
      std::uint32_t sectionSize_briefdata;
      std::uint32_t sectionSize_fulldata;//on-disk, not uncompressed
      std::streampos evtPosInFile;
      std::uint32_t evtIndex;
      std::uint32_t dummy;//Here to have sizeof(EventInfo)==8*sizeof(std::uint32_t)+sizeof(streampos) even on 64bit.
    };
    static_assert( sizeof(EventInfo)==8*sizeof(std::uint32_t)+sizeof(std::streampos), "" );
    EventInfo * m_currentEventInfo = nullptr;
    std::vector<EventInfo> m_evts;
    ////The next map is only populated on demand when goToEvent is called!
    std::map<std::pair<std::uint32_t,std::uint32_t>,EventInfo* > m_evtMap; //(runNbr,evtNbr) -> EventInfo*
    std::string m_fileName;
    std::map<unsigned,IDBSubSectionReader*> m_dbsubsects;

  };

}

#include "EvtFile/FileReader.icc"

#endif
