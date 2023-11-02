#include "EvtFile/DBSubSectReaderMgr.hh"
#include "Utils/ByteStream.hh"
#include <cassert>

void EvtFile::DBSubSectReaderMgr::addSubSection(IDBSubSectionReader&s)
{
  unsigned id = s.uniqueSubSectionID();
  assert(m_subSections.find(id)==m_subSections.end()&&"Sub-section IDs must be unique!");
  m_subSections[id]=&s;
}

void EvtFile::DBSubSectReaderMgr::clearInfo()
{
  auto itE=m_subSections.end();
  for (auto it=m_subSections.begin();it!=itE;++it)
    it->second->clearInfo();
}

void EvtFile::DBSubSectReaderMgr::newInfoAvailable(const char*data, unsigned nbytes)
{
  //Distribute the received data out to the sub section readers
  std::uint16_t subsectid_raw;
  unsigned subsectid;
  const char* dataE(data+nbytes);
  while(data<dataE)
    {
      assert(dataE-data>=int(sizeof(std::uint16_t)));
      ByteStream::read(data,subsectid_raw);
      subsectid=subsectid_raw;
      auto it = m_subSections.find(subsectid);
      if (it!=m_subSections.end())
        {
          it->second->load(data);
          assert(data<=dataE);//can't read past the end
        }
      else
        {
          printf("ERROR: no reader for db subsection with id %i\n",subsectid);
          assert(false&&"no reader for db subsection");
        }
    }
}
