#include "EvtFile/FileWriter.hh"
#include "EvtFile/DBStringsWriter.hh"
#include "EvtFile/DBEntryWriter.hh"
#include "EvtFile/DBEntryReader.hh"

#include "EvtFile/FileReader.hh"
#include "EvtFile/DBSubSectReaderMgr.hh"
#include "EvtFile/DBStringsReader.hh"
#include "Utils/ByteStream.hh"

#include "EvtFile/DumpFile.hh"
#include <cstdio>
#include <string>

namespace {
  class DummyFormat : public EvtFile::IFormat {
  public:
    DummyFormat() {}
    std::uint32_t magicWord() const { return 0x87654321; }
    const char* fileExtension() const { return ".dmy2"; }
    const char* eventBriefDataName() const { return "header"; }
    const char* eventFullDataName() const { return "details"; }
  };

  static const DummyFormat dummyFormat;

  class CustomWritableEntry : public EvtFile::IDBEntry {

  public:
    CustomWritableEntry(std::uint16_t v1,
                        std::uint16_t v2,
                        const std::string& s,
                        EvtFile::index_type name)
      : IDBEntry(), m_val1(v1), m_val2(v2), m_str(s), m_nameIdx(name) {}

    virtual void write(EvtFile::FileWriter&fw)
    {
      fw.writeDataDBSection(m_val1);
      fw.writeDataDBSection(m_val2);
      assert(m_str.size()<UINT16_MAX);
      fw.writeDataDBSection((std::uint16_t)m_str.size());
      if (!m_str.empty())
        fw.writeDataDBSection(&(m_str[0]),m_str.size());
      fw.writeDataDBSection(m_nameIdx);
    }
    std::uint16_t val1() const { return m_val1; }
    std::uint16_t val2() const { return m_val2; }
    const std::string& str() const { return m_str; }
    EvtFile::index_type nameIdx() const { return m_nameIdx; }

  protected:

    virtual unsigned calculateHash() const
    {
      //on purpose we make a really shitty hash here (and we make sure below to
      //have clashes):
      return (unsigned)(m_val1);//anything with similar m_val1 will clash!!
    };

    virtual bool lessThan(const IDBEntry& oe) const
    {
      const CustomWritableEntry*o= static_cast<const CustomWritableEntry*>(&oe);
      if (m_val1!=o->m_val1) return m_val1<o->m_val1;
      if (m_val2!=o->m_val2) return m_val2<o->m_val2;
      if (m_nameIdx!=o->m_nameIdx) return m_nameIdx<o->m_nameIdx;
      return m_str<o->m_str;
    }

    virtual bool equals(const IDBEntry& oe) const
    {
      const CustomWritableEntry*o= static_cast<const CustomWritableEntry*>(&oe);
      return m_val1==o->m_val1 && m_val2==o->m_val2 && m_str==o->m_str;
    }

  private:
    std::uint16_t m_val1;
    std::uint16_t m_val2;
    std::string m_str;//directly written string
    EvtFile::index_type m_nameIdx;//string kept in another DB section
  };

  class CustomReadableEntry {

  public:
    CustomReadableEntry(const char*&data)
    {
      ByteStream::read(data,m_val1);
      ByteStream::read(data,m_val2);
      ByteStream::read(data,m_str);
      ByteStream::read(data,m_nameIdx);
    }

    std::uint16_t val1() const { return m_val1; }
    std::uint16_t val2() const { return m_val2; }
    const std::string& str() const { return m_str; }
    EvtFile::index_type nameIdx() const { return m_nameIdx; }

    const EvtFile::str_type& name(const EvtFile::DBStringsReader&sr) const
    {
      return sr.getString(m_nameIdx);
    }

  private:
    std::uint16_t m_val1;
    std::uint16_t m_val2;
    std::string m_str;//directly written string
    EvtFile::index_type m_nameIdx;//string kept in another DB section
  };






  unsigned iwrite = 0;
  void flush(EvtFile::FileWriter&fw,unsigned run,unsigned evt)
  {
    printf("Flushing writing: idx %i runnbr %i evtnbr %i\n",iwrite++,run,evt);
    fw.flushEventToDisk(run,evt);
  }

  typedef std::pair<EvtFile::index_type,EvtFile::index_type> AddressType;

  void addCustom(EvtFile::FileWriter&fw,
                 EvtFile::DBEntryWriter&ew ,
                 EvtFile::DBStringsWriter& sw,
                 std::uint16_t a,std::uint16_t b,const char* c,const char * name)
  {
    CustomWritableEntry * ce = new CustomWritableEntry(a,b,c,sw.getIndex(name));
    ce->ref();
    fw.writeDataFullSection(ew.getIndex(ce));
    ce->unref();
  }

  int writetest()
  {
    EvtFile::FileWriter fw(&dummyFormat,"testdata");

    if (!fw.ok()) {
      printf("Error: Failed to open output file\n");
      return 1;
    }

    EvtFile::DBStringsWriter names(100,fw);
    EvtFile::DBStringsWriter cities(200,fw);
    EvtFile::DBEntryWriter custom(300,fw);

    ///////////////////////// Event #1 //////////////////////////

    fw.writeDataBriefSection(AddressType(names.getIndex("Joe"),cities.getIndex("New York")));
    fw.writeDataBriefSection(AddressType(names.getIndex("William"),cities.getIndex("London")));
    fw.writeDataBriefSection(AddressType(names.getIndex("Olivier"),cities.getIndex("Paris")));

    addCustom(fw,custom,names,1,2,"test1","Will");
    addCustom(fw,custom,names,1,2,"test1","Will");
    addCustom(fw,custom,names,1,3,"test1","Will");
    addCustom(fw,custom,names,1,2,"test2","Will");
    addCustom(fw,custom,names,1,2,"test1","Bill");
    addCustom(fw,custom,names,1,2,"test1","John");

    flush(fw,1000,1);

    ///////////////////////// Event #2 //////////////////////////

    //similar to event 1 but cities swapped => should add nothing in the DB section
    fw.writeDataBriefSection(AddressType(names.getIndex("Olivier"),cities.getIndex("London")));
    fw.writeDataBriefSection(AddressType(names.getIndex("William"),cities.getIndex("New York")));
    fw.writeDataBriefSection(AddressType(names.getIndex("Joe"),cities.getIndex("Paris")));

    addCustom(fw,custom,names,1,2,"test1","Will");
    addCustom(fw,custom,names,2,2,"test1","Will");
    addCustom(fw,custom,names,3,3,"test1","Will");
    addCustom(fw,custom,names,1,2,"test2","Will");
    addCustom(fw,custom,names,2,2,"test1","Bill");
    addCustom(fw,custom,names,3,2,"test1","John");

    flush(fw,1000,2);

    ///////////////////////// Event #3 //////////////////////////

    //
    addCustom(fw,custom,names,1,2,"Olivier","Olivier");
    addCustom(fw,custom,names,2,2,"test1","William");
    addCustom(fw,custom,names,3,3,"Paris","Dundee");
    addCustom(fw,custom,names,1,2,"test2","Will");
    addCustom(fw,custom,names,2,2,"William","Bill");
    addCustom(fw,custom,names,3,2,"test1","John");

    //Some new, some old stuff:
    fw.writeDataBriefSection(AddressType(names.getIndex("Olivier"),cities.getIndex("New York")));
    fw.writeDataBriefSection(AddressType(names.getIndex("Dundee"),cities.getIndex("Sydney")));
    fw.writeDataBriefSection(AddressType(names.getIndex("Paris"),cities.getIndex("Copenhagen")));

    flush(fw,1000,3);

    if (!fw.ok()) {
      printf("Error: Unexpected problems while writing\n");
      return 1;
    }
    return 0;
  }

  int readtest(const char * filename)
  {
    EvtFile::DBStringsReader names(100);
    EvtFile::DBStringsReader cities(200);
    EvtFile::DBEntryReader<CustomReadableEntry> custom(300);
    EvtFile::DBSubSectReaderMgr db;
    db.addSubSection(names);
    db.addSubSection(cities);
    db.addSubSection(custom);
    EvtFile::FileReader fr(&dummyFormat,filename,&db);

    if (!fr.init()) {
      printf("Error: Failed to open input file: %s\n",fr.bad_reason());
      return 1;
    }

    while(fr.eventActive()) {
      printf("Found event at idx %i: runnbr %i evtnbr %i nbytes_briefdata %i nbytes_fulldata %i\n",
             fr.eventIndex(),fr.runNumber(),fr.eventNumber(),fr.nBytesBriefData(),fr.nBytesFullDataOnDisk());

      int nb(fr.nBytesBriefData());
      const char* data(fr.getBriefData());
      while (nb>0) {
        AddressType a = *(reinterpret_cast<const AddressType*>(data));
        data+=sizeof(AddressType);
        nb-=sizeof(AddressType);
        printf("   => Found content in brief section: (\"%s\", \"%s\")\n",names.getString(a.first).c_str(),cities.getString(a.second).c_str());
      }

      const char* dataf(fr.getFullData());
      const char* datafE(fr.getFullData()+fr.nBytesFullDataOnDisk());
      while (dataf<datafE) {
        EvtFile::index_type idx;
        ByteStream::read(dataf,idx);

        const CustomReadableEntry& ce = custom.getEntry( idx );
        printf("   => Found content in Full section: (%i, %i, \"%s\", \"%s\")\n",ce.val1(),ce.val2(),ce.str().c_str(),ce.name(names).c_str());
      }
      fr.goToNextEvent();
    }

    return 0;
  }
}

int main(int,char**)
{
  printf("=====> Write file testdata.dmy2\n");

  int ec=writetest();
  if (ec)
    return ec;

  printf("=====> Standard run through file\n");

  ec=readtest("testdata.dmy2");
  if (ec)
    return ec;

  printf("=====> File dump\n");

  if (!EvtFile::dumpFileInfo(&dummyFormat,"testdata.dmy2"))
    return 1;

  printf("=====> Finished without encountering error conditions.\n");
  return 0;
}

