#ifndef EvtFile_DBSubSectReaderMgr_hh
#define EvtFile_DBSubSectReaderMgr_hh

#include "EvtFile/FileReader.hh"

namespace EvtFile {

  class IDBSubSectionReader;

  class DBSubSectReaderMgr : public EvtFileDB {
  public:
    DBSubSectReaderMgr(){}
    virtual ~DBSubSectReaderMgr(){}

    void addSubSection(IDBSubSectionReader&);//will keep a reference to the subsection

  private:

    DBSubSectReaderMgr( const DBSubSectReaderMgr & );
    DBSubSectReaderMgr & operator= ( const DBSubSectReaderMgr & );

    void newInfoAvailable(const char*data, unsigned nbytes);
    void clearInfo();

    std::map<unsigned,IDBSubSectionReader*> m_subSections;

  };

}

#endif
