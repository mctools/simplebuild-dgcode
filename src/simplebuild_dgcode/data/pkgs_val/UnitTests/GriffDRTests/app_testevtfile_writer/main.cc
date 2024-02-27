#include "EvtFile/FileWriter.hh"
#include "EvtFile/FileReader.hh"
#include <cstdio>
#include <string>
//#include "G4String.hh"

namespace {
  class DummyFormat : public EvtFile::IFormat {
  public:
    DummyFormat() {}
    std::uint32_t magicWord() const { return 0x12345678; }
    const char* fileExtension() const { return ".dmy"; }
    const char* eventBriefDataName() const { return "brf"; }
    const char* eventFullDataName() const { return "dtld"; }
  };

  static const DummyFormat dummyFormat;

  unsigned iwrite = 0;
  void flush(EvtFile::FileWriter&fw,unsigned run,unsigned evt)
  {
    printf("Flushing writing: idx %i runnbr %i evtnbr %i\n",iwrite++,run,evt);
    fw.flushEventToDisk(run,evt);
  }

  int writetest()
  {
    EvtFile::FileWriter fw(&dummyFormat,"testdata");

    if (!fw.ok()) {
      printf("Error: Failed to open output file\n");
      return 1;
    }
    ///////////////////////// Event #1 //////////////////////////

    fw.writeDataDBSection((int32_t)0x1);
    fw.writeDataDBSection((int32_t)0x2);
    fw.writeDataDBSection((int32_t)0x3);

    fw.writeDataFullSection((int64_t)0x111);
    fw.writeDataFullSection((int64_t)0x222);
    fw.writeDataFullSection((int64_t)0x333);

    fw.writeDataBriefSection((int64_t)0x11);
    fw.writeDataBriefSection((int64_t)0x22);
    fw.writeDataBriefSection((int64_t)0x33);

    fw.writeDataDBSection((int16_t)0x9999);

    flush(fw,1000,1);

    ///////////////////////// Event #2 //////////////////////////

    fw.writeDataBriefSection((int64_t)0x44);

    fw.writeDataFullSection((int64_t)0x444);
    fw.writeDataFullSection((int64_t)0x555);
    fw.writeDataFullSection((int64_t)0x666);

    fw.writeDataDBSection((int16_t)0x8888);

    flush(fw,1000,2);

    ////////////////////////// 5 events with simple info with non-consequtive evt numbers //////////
    for(unsigned i=0;i<5;++i) {
      fw.writeDataBriefSection((int32_t)i);
      fw.writeDataFullSection((int32_t)i);
      if (i%2==0)
        fw.writeDataDBSection((int32_t)i);
      flush(fw,1000,3+i*2);
    }

    /////////////////////////  5 empty events from a different run ///////////////////////
    for(unsigned i=0;i<5;++i) {
      if (i%2==0)
        fw.writeDataDBSection((int32_t)i+100);
      flush(fw,1100,i);
    }

    if (!fw.ok()) {
      printf("Error: Unexpected problems while writing\n");
      return 1;
    }
    return 0;
  }

  class TestDBListener : public EvtFile::EvtFileDB {
  public:
    virtual void newInfoAvailable(const char*/*data*/, unsigned nbytes)
    {
      printf("DB Listener: Received %i bytes of data\n",nbytes);
      nbytesreceived += nbytes;
    }
    virtual void clearInfo()
    {
      //Could do:
      //  nbytesreceived = 0;
      //but we just want to keep the record of all of it for afterwards
    }

    TestDBListener() : EvtFile::EvtFileDB(), nbytesreceived(0)
    {
    }

    ~TestDBListener()
    {
      printf("TestDBListener destructor: Received a total of %i bytes\n",nbytesreceived);
    }
    unsigned nbytesreceived;
  };

  void printevt(const EvtFile::FileReader&fr)
  {
    if (fr.eventActive()) {
      printf("Found event at idx %i: runnbr %i evtnbr %i nbytes_briefdata %i nbytes_fulldata %i\n",
             fr.eventIndex(),fr.runNumber(),fr.eventNumber(),fr.nBytesBriefData(),fr.nBytesFullDataOnDisk());
    } else {
      printf("Did not find event\n");
    }
  }


  int readtest()
  {
    TestDBListener db;
    EvtFile::FileReader fr(&dummyFormat,"testdata.dmy",&db);

    if (!fr.init()) {
      printf("Error: Failed to open input file: %s\n",fr.bad_reason());
      return 1;
    }

    while(fr.eventActive()) {
      printevt(fr);
      fr.goToNextEvent();
    }

    return 0;
  }

  int readtest_jumparound()
  {
    TestDBListener db;
    EvtFile::FileReader fr(&dummyFormat,"testdata.dmy",&db);

    if (!fr.init()) {
      printf("Error: Failed to open input file: %s\n",fr.bad_reason());
      return 1;
    }
    printf("skipEvents(8)\n");
    fr.skipEvents(8);
    printevt(fr);
    printf("goToNextEvent()\n");
    fr.goToNextEvent();
    printevt(fr);
    printf("skipEvents(-7)\n");
    fr.skipEvents(-7);
    printevt(fr);

    fr.goToNextEvent();
    printf("run forward through events...\n");
    while(fr.eventActive()) {
      printevt(fr);
      fr.goToNextEvent();
    }
    printf("goToFirstEvent()\n");
    fr.goToFirstEvent();
    printevt(fr);
    printf("seekEventByIndex(8) and run forward through events...\n");
    fr.seekEventByIndex(8);
    while(fr.eventActive()) {
      printevt(fr);
      fr.goToNextEvent();
    }
    printf("gotoEvent(1100,2)\n");
    fr.goToEvent(1100,2);
    printevt(fr);
    return 0;
  }
}

int main(int,char**)
{
  printf("=====> Write file testdata.dmy\n");

  int ec=writetest();
  if (ec)
    return ec;

  printf("=====> Standard run through file\n");

  ec=readtest();
  if (ec)
    return ec;

  printf("=====> Read with a bit of jumping around\n");

  ec=readtest_jumparound();
  if (ec)
    return ec;

  printf("=====> Finished without encountering error conditions.\n");
  return 0;
}

