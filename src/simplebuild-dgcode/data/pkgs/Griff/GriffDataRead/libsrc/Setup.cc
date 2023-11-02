#include "GriffDataRead/Setup.hh"
#include "Utils/ByteStream.hh"
#include <set>

GriffDataRead::Setup::~Setup()
{
  delete m_geo;
  delete m_gen;
  delete m_filter;
  delete m_killFilter;
}

void GriffDataRead::Setup::initBinaryData() const
{
  assert(!m_binaryDataInit);
  m_binaryDataInit=true;
  std::string tmp;
  auto itE = m_allData.end();
  for (auto it = m_allData.begin(); it!=itE; ++it) {
    assert(!it->first.empty());
    if (it->first[0]!='`')
      continue;
    assert(it->first.size()>=2);
    tmp.assign(&(it->first[1]),it->first.size()-1);
    m_binaryData[tmp]=it->second;
  }
}

void GriffDataRead::Setup::initUserData() const
{
  assert(!m_userDataInit);
  m_userDataInit=true;
  std::string tmp;
  auto itE = m_allData.end();
  for (auto it = m_allData.begin(); it!=itE; ++it) {
    assert(!it->first.empty());
    if (it->first[0]!='^')
      continue;
    assert(it->first.size()>=2);
    tmp.assign(&(it->first[1]),it->first.size()-1);
    m_userData[tmp]=it->second;
  }
}

void GriffDataRead::Setup::initMetaData() const
{
  assert(!m_metaDataInit);
  m_metaDataInit=true;
  auto itE = m_allData.end();
  for (auto it = m_allData.begin(); it!=itE; ++it) {
    assert(!it->first.empty());
    if (it->first[0]=='^'||it->first[0]=='`')//binary or user data
      continue;
    m_metaData.insert(m_metaData.end(),*it);
    //The .end() hint is in C++11 how we tell it that we expect the insert to be
    //at the end. In C++98 we would have had to point at the previous insert for
    //best performance.
  }
}

void GriffDataRead::Setup::initCmds() const
{
  assert(!m_cmdsInit);
  m_cmdsInit=true;
  assert(m_cmds.empty());
  auto m = binaryData();
  auto it = m.find("g4Cmds");
  if (it==m.end())
    return;
  const char * data = it->second.c_str();
  ByteStream::read(data,m_cmds);
}

void GriffDataRead::Setup::initGeo() const
{
  assert(!m_geoInit);
  assert(!m_geo);
  m_geoInit=true;
  if (!m_binaryDataInit)
    initBinaryData();
  auto itdata = m_binaryData.find("geoSerialised");
  if (itdata==m_binaryData.end())
    return;
  if (!m_metaDataInit)
    initMetaData();
  auto itname = m_metaData.find("geoName");
  if (itname==m_metaData.end())
    return;
  m_geo = new GeoParams(itdata->second.c_str(),itname->second);
}

void GriffDataRead::Setup::initGen() const
{
  assert(!m_genInit);
  assert(!m_gen);
  m_genInit=true;
  if (!m_binaryDataInit)
    initBinaryData();
  auto itdata = m_binaryData.find("genSerialised");
  if (itdata==m_binaryData.end())
    return;
  if (!m_metaDataInit)
    initMetaData();
  auto itname = m_metaData.find("genName");
  if (itname==m_metaData.end())
    return;
  m_gen = new GenParams(itdata->second.c_str(),itname->second);
}

void GriffDataRead::Setup::initFilter() const
{
  assert(!m_filterInit);
  assert(!m_filter);
  m_filterInit=true;
  if (!m_binaryDataInit)
    initBinaryData();
  auto itdata = m_binaryData.find("filterSerialised");
  if (itdata==m_binaryData.end())
    return;
  if (!m_metaDataInit)
    initMetaData();
  auto itname = m_metaData.find("filterName");
  if (itname==m_metaData.end())
    return;
  m_filter = new FilterParams(itdata->second.c_str(),itname->second);
}

void GriffDataRead::Setup::initKillFilter() const
{
  assert(!m_killFilterInit);
  assert(!m_killFilter);
  m_killFilterInit=true;
  if (!m_binaryDataInit)
    initBinaryData();
  auto itdata = m_binaryData.find("killFilterSerialised");
  if (itdata==m_binaryData.end())
    return;
  if (!m_metaDataInit)
    initMetaData();
  auto itname = m_metaData.find("killFilterName");
  if (itname==m_metaData.end())
    return;
  m_killFilter = new FilterParams(itdata->second.c_str(),itname->second);
}

void GriffDataRead::Setup::dump(const char* prefix) const
{
  printf("%s===========================  GRIFF SETUP  ===========================\n",prefix);
  {
    printf("%s  UserData:\n",prefix);
    auto m = userData();
    if (m.empty()) {
      printf("%s    <none>\n",prefix);
    } else {
      std::set<std::string> keysorted;
      auto itE = m.end();
      for (auto it = m.begin();it!=itE;++it)
        keysorted.insert(it->first);
      auto itSE = keysorted.end();
      for (auto itS = keysorted.begin();itS!=itSE;++itS)
        printf("%s    \"%s\" : \"%s\"\n",prefix,itS->c_str(),m[*itS].c_str());
    }
  }
  {
    printf("%s  MetaData:\n",prefix);
    auto m = metaData();
    if (m.empty()) {
      printf("%s    <none>\n",prefix);
    } else {
      std::set<std::string> keysorted;
      auto itE = m.end();
      for (auto it = m.begin();it!=itE;++it)
        keysorted.insert(it->first);
      auto itSE = keysorted.end();
      for (auto itS = keysorted.begin();itS!=itSE;++itS)
        printf("%s    \"%s\" : \"%s\"\n",prefix,itS->c_str(),m[*itS].c_str());
    }
  }
  {
    printf("%s  Geant4 Commands:\n",prefix);
    auto m = cmds();
    if (m.empty()) {
      printf("%s    <none>\n",prefix);
    } else {
      auto itE = m.end();
      for (auto it = m.begin();it!=itE;++it)
        printf("%s    \"%s\"\n",prefix,it->c_str());
    }
  }
  {
    std::string tmp(prefix);
    tmp+="  ";
    geo().dump(tmp.c_str());
    gen().dump(tmp.c_str());
    if (hasFilter())
      filter().dump(tmp.c_str());
    if (hasKillFilter()) {
      printf("%sKillFilter:\n",tmp.c_str());
      tmp+="  ";
      killFilter().dump(tmp.c_str());
    }
  }
  printf("%s=====================================================================\n",prefix);
}

GriffDataRead::GeoParams::GeoParams(const char* data, const std::string& name)
  : DummyParamHolder(data), m_name(name)
{
}

GriffDataRead::GenParams::GenParams(const char* data, const std::string& name)
  : DummyParamHolder(data), m_name(name)
{
}

GriffDataRead::FilterParams::FilterParams(const char* data, const std::string& name)
  : DummyParamHolder(data), m_name(name)
{
}

void GriffDataRead::GeoParams::dump(const char* prefix) const
{
  printf("%sGeoConstructor[%s]:\n",prefix,m_name.c_str());
  std::string p = prefix;
  p+="  ";
  Utils::DummyParamHolder::dump(p.c_str());
}

void GriffDataRead::GenParams::dump(const char* prefix) const
{
  printf("%sParticleGenerator[%s]:\n",prefix,m_name.c_str());
  std::string p = prefix;
  p+="  ";
  Utils::DummyParamHolder::dump(p.c_str());
}

void GriffDataRead::FilterParams::dump(const char* prefix) const
{
  printf("%sStepFilter[%s]:\n",prefix,m_name.c_str());
  std::string p = prefix;
  p+="  ";
  Utils::DummyParamHolder::dump(p.c_str());
}

