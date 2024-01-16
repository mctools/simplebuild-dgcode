#ifndef GriffDataRead_Setup_hh
#define GriffDataRead_Setup_hh

#include "Utils/DummyParamHolder.hh"
#include "Utils/RefCountBase.hh"
#include "Utils/StringSort.hh"
#include <string>
#include <map>

class GriffDataReader;
namespace GriffDataRead
{
  class GeoParams;
  class GenParams;
  class FilterParams;
  typedef std::map<std::string,std::string,Utils::fast_str_cmp> StrMap;
  typedef std::vector<std::string> StrVect;

  class Setup : public Utils::RefCountBase {
  public:
    ///////////////////////////////////////////////////////////////////
    //  Various meta-data describing the setup of the simulation job //
    ///////////////////////////////////////////////////////////////////

    const GeoParams& geo() const;//named parameter object with geometry settings
    const GenParams& gen() const;//named parameter object with generator settings
    bool hasFilter() const;//check this before calling filter() below
    const FilterParams& filter() const;//named parameter object with filter settings
    bool hasKillFilter() const;//check this before calling killFilter() below
    const FilterParams& killFilter() const;//named parameter object with filter settings

    const StrVect& cmds() const;//Geant4 interactive commands

    const StrMap& userData() const;//custom metadata
    const StrMap& metaData() const;//standard metadata

    const std::string& metaData(const std::string&key) const { return metaData().find(key)->second; }

    void dump(const char* prefix="") const;

    /////////////////////////////
    //  Implementation Details //
    /////////////////////////////
  private:
    friend class ::GriffDataReader;
    const StrMap& allData() const;
    const StrMap& binaryData() const;
    virtual ~Setup();
    Setup(const StrMap&);
    StrMap m_allData;
    mutable StrMap m_userData;
    mutable StrMap m_metaData;
    mutable StrMap m_binaryData;
    mutable StrVect m_cmds;
    mutable GeoParams * m_geo;
    mutable GenParams * m_gen;
    mutable FilterParams * m_filter;
    mutable FilterParams * m_killFilter;
    mutable bool m_userDataInit;
    mutable bool m_metaDataInit;
    mutable bool m_binaryDataInit;
    mutable bool m_geoInit;
    mutable bool m_genInit;
    mutable bool m_filterInit;
    mutable bool m_killFilterInit;
    mutable bool m_cmdsInit;
    void initBinaryData() const;
    void initUserData() const;
    void initMetaData() const;
    void initGeo() const;
    void initGen() const;
    void initFilter() const;
    void initKillFilter() const;
    void initCmds() const;
  };

  class GeoParams : public Utils::DummyParamHolder {
  public:
    const std::string& getName() const;
    const char* getNameCStr() const { return getName().c_str(); }
    virtual void dump(const char* prefix="") const;
  private:
    friend class Setup;
    std::string m_name;
    GeoParams(const char* data, const std::string& name);
    virtual ~GeoParams(){}
  };
  class GenParams : public Utils::DummyParamHolder {
  public:
    const std::string& getName() const;
    const char* getNameCStr() const { return getName().c_str(); }
    virtual void dump(const char* prefix="") const;
  private:
    friend class Setup;
    std::string m_name;
    GenParams(const char* data, const std::string& name);
    virtual ~GenParams(){}
  };
  class FilterParams : public Utils::DummyParamHolder {
  public:
    const std::string& getName() const;
    const char* getNameCStr() const { return getName().c_str(); }
    virtual void dump(const char* prefix="") const;
  private:
    friend class Setup;
    std::string m_name;
    FilterParams(const char* data, const std::string& name);
    virtual ~FilterParams(){}
  };

}

#include "GriffDataRead/Setup.icc"

#endif
