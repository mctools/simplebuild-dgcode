#ifndef GriffDataRead_MetaData_hh
#define GriffDataRead_MetaData_hh

#include "EvtFile/DBEntryReader.hh"
#include "EvtFile/Defs.hh"
#include "Utils/StringSort.hh"
#include <vector>
#include <map>
#include <string>

class GriffDataReader;

namespace GriffDataRead {

  class Material;

  class MetaData {
  public:
    const std::map<std::string,std::string,Utils::fast_str_cmp>& getMap(GriffDataReader* dr) const
    {
      if (m_data.size()>m_cache.size())
        initCache(dr);
      return m_cache;
    }
  private:
    friend class EvtFile::DBEntryReader<MetaData>;
    MetaData(const char*&data);
    mutable std::vector<std::pair<EvtFile::index_type,EvtFile::index_type> > m_data;
    void initCache(GriffDataReader* dr) const;
    mutable std::map<std::string,std::string,Utils::fast_str_cmp> m_cache;
  };

}

#endif
