#ifndef EvtFile_DBSubSectReaderMgr_hh
#define EvtFile_DBSubSectReaderMgr_hh

#include "EvtFile/FileReader.hh"

namespace EvtFile {

  class IDBSubSectionReader;

  class DBSubSectReaderMgr final : public EvtFileDB {
  public:
    DBSubSectReaderMgr(){}
    virtual ~DBSubSectReaderMgr(){}

    DBSubSectReaderMgr( const DBSubSectReaderMgr& ) = delete;
    DBSubSectReaderMgr& operator=( const DBSubSectReaderMgr& ) = delete;
    DBSubSectReaderMgr( DBSubSectReaderMgr&& ) = delete;
    DBSubSectReaderMgr& operator=( DBSubSectReaderMgr&& ) = delete;

    void addSubSection(IDBSubSectionReader&);//will keep a reference to the subsection

  private:

    void newInfoAvailable(const char*data, unsigned nbytes);
    void clearInfo();

    std::map<unsigned,IDBSubSectionReader*> m_subSections;

  };

}

#endif
