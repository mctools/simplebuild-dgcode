#include "EvtFile/DumpFile.hh"
#include "EvtFileDefs.hh"
#include "EvtFile/FileReader.hh"
#include <cassert>
#include "Core/Types.hh"
#include <cstdio>
#include <sstream>

namespace EvtFile {

  class DumpFileDBListener : public EvtFileDB {
  public:
    DumpFileDBListener(bool dumpData=false)
      : EvtFileDB(), m_nbytes(0), m_dumpData(dumpData)
    {
    }

    void setDumpData(bool b)
    {
      m_dumpData=b;
    }

    virtual ~DumpFileDBListener(){}

    virtual void newInfoAvailable(const char* data, unsigned nbytes)
    {
      m_nbytes=nbytes;
      if (m_dumpData && nbytes) {
        for (unsigned i=0; i<nbytes; ++i)
          putchar(data[i]);
      }
    }

    virtual void clearInfo()
    {
      m_nbytes = 0;
    }

    unsigned int nBytesReceived() const { return m_nbytes; }
    void clear() { m_nbytes=0; }
  private:
    unsigned m_nbytes;
    bool m_dumpData;
  };

  bool dumpFileInfo(const IFormat* format, const char* filename, bool brief, bool uncompressed_sizes)
  {
    printf("Dumping %s:\n",filename);
    DumpFileDBListener db(false);
    FileReader f(format,filename,&db);
    f.init();
    if (f.bad())
      {
        printf("  Error: %s\n",f.bad_reason());
        return false;
      }
    if (!f.is_open())
      {
        printf("  Error: Could not open file\n");
        return false;
      }
    printf("  File format version: %i\n",f.version());
    if (!f.eventActive()) {
      printf("  No events in file.\n");
      return true;//not an error!
    }

    unsigned nevts(0);

    printf("  Position RunNbr EvtNbr EvtHdr[B] DBData[B] BriefData[B] FullData[B] Total[B] Integrity\n");
    std::uint64_t totdb(0),totbrief(0),totfull(0);
    bool badEvents(false);
    while (f.eventActive()) {
      ++nevts;
      if (!f.ok())
        {
          printf("  Error: %s\n",f.bad_reason());
          return false;
        }
      bool integrity(f.verifyEventDataIntegrity());
      unsigned fulldatasize = uncompressed_sizes ? f.nBytesFullData() : f.nBytesFullDataOnDisk();
      if (!brief||!integrity) {
        std::ostringstream stmp;stmp<<std::uint64_t(EVTFILE_EVENT_HEADER_BYTES+db.nBytesReceived()+f.nBytesBriefData()+fulldatasize);

        printf("  %8i %6i %6i %9i %9i %12i %11i %8s %s\n",
               f.eventIndex(),f.runNumber(),f.eventNumber(),
               int(EVTFILE_EVENT_HEADER_BYTES), db.nBytesReceived(),f.nBytesBriefData(),fulldatasize,
               stmp.str().c_str(),
               integrity ? "[success]" : "[failure]" );
      }
      if (!integrity)
        badEvents=true;

      totdb+=db.nBytesReceived();
      totbrief+=f.nBytesBriefData();
      totfull+=fulldatasize;
      db.clear();
      f.goToNextEvent();
    };

    std::ostringstream s;
    s<<"Total [nevts="<<nevts<<"]:";
    std::ostringstream stmp1;stmp1<<std::uint64_t(nevts*EVTFILE_EVENT_HEADER_BYTES);
    std::ostringstream stmp2;stmp2<<totdb;
    std::ostringstream stmp3;stmp3<<totbrief;
    std::ostringstream stmp4;stmp4<<totfull;
    std::ostringstream stmp5;stmp5<<std::uint64_t(nevts*EVTFILE_EVENT_HEADER_BYTES)+totdb+totbrief+totfull;

    printf("  %-22s %9s %9s %12s %11s %8s %s\n",
           s.str().c_str(),
           stmp1.str().c_str(),
           stmp2.str().c_str(),
           stmp3.str().c_str(),
           stmp4.str().c_str(),
           stmp5.str().c_str(),
           badEvents ? "[failure]" : "[success]");

    if (badEvents) {
      printf("  ERROR: Not all events passed a data integrity check. File appears to be corrupted!\n");
      return false;
    }

    return true;
  }

  bool dumpFileEventDBSection(const IFormat* format, const char* filename, unsigned evtIndex )
  {
    DumpFileDBListener db(evtIndex==0);
    FileReader f(format,filename,&db);
    f.init();
    if (f.bad())
      {
        fprintf(stderr,"Error: %s\n",f.bad_reason());
        return false;
      }
    if (!f.is_open())
      {
        fprintf(stderr,"  Error: Could not open file\n");
        return false;
      }
    if (!f.eventActive()) {
      fprintf(stderr,"  Error: Could not find event at index %i\n",evtIndex);
      return false;
    }
    if (evtIndex>0) {
      if (!f.seekEventByIndex(evtIndex-1)) {
        fprintf(stderr,"  Error: Could not find event at index %i\n",evtIndex);
        return false;
      }
      db.setDumpData(true);
      if (!f.seekEventByIndex(evtIndex)) {
        printf("  Error: Could not find event at index %i\n",evtIndex);
        return false;
      }
    }
    return true;
  }

  bool dumpFileEventBriefDataSection(const IFormat* format, const char* filename, unsigned evtIndex)
  {
    FileReader f(format,filename,0);
    f.init();
    if (f.bad())
      {
        fprintf(stderr,"Error: %s\n",f.bad_reason());
        return false;
      }
    if (!f.is_open())
      {
        fprintf(stderr,"  Error: Could not open file\n");
        return false;
      }
    if (evtIndex>0) {
      if (!f.seekEventByIndex(evtIndex)) {
        printf("  Error: Could not find event at index %i\n",evtIndex);
        return false;
      }
    }
    if (!f.eventActive()) {
      fprintf(stderr,"  Error: Could not find event at index %i\n",evtIndex);
      return false;
    }

    unsigned n=f.nBytesBriefData();
    if (n) {
      const char * data = f.getBriefData();
      for (unsigned i=0; i<f.nBytesBriefData(); ++i)
        putchar(data[i]);
    }
    return true;
  }

  bool dumpFileEventFullDataSection(const IFormat* format, const char* filename, unsigned evtIndex)
  {
    FileReader f(format,filename,0);
    f.init();
    if (f.bad())
      {
        fprintf(stderr,"Error: %s\n",f.bad_reason());
        return false;
      }
    if (!f.is_open())
      {
        fprintf(stderr,"  Error: Could not open file\n");
        return false;
      }
    if (evtIndex>0) {
      if (!f.seekEventByIndex(evtIndex)) {
        printf("  Error: Could not find event at index %i\n",evtIndex);
        return false;
      }
    }
    if (!f.eventActive()) {
      fprintf(stderr,"  Error: Could not find event at index %i\n",evtIndex);
      return false;
    }

    unsigned n=f.nBytesFullDataOnDisk();
    if (n) {
      const char * data = f.getFullData();
      for (unsigned i=0; i<f.nBytesFullDataOnDisk(); ++i)
        putchar(data[i]);
    }
    return true;
  }


}
