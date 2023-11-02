#include "Core/Types.hh"
#include "Utils/ParametersBase.hh"
#include "Utils/StringSort.hh"
#include "Utils/ByteStream.hh"
#include <cassert>
#include "Core/String.hh"
#include <map>
#include <set>
#include <list>
#include <vector>
#include <cassert>
#include <iostream>
#include <cstdio>
#include <stdexcept>

struct Utils::ParametersBase::Imp {
  Imp(ParametersBase*pb) : m_locked(false), m_validating(false), m_ignoreRanges(false), m_hardExit(true), m_pb(pb) {}
  ~Imp()
  {
    //Remove any ties in other ParametersBase objects, to avoid them calling us later.
    untie(m_mapDouble);
    untie(m_mapInt);
    untie(m_mapBool);
    untie(m_mapStr);
  }

  typedef std::list<std::pair<ParametersBase*,std::string> > TieList;

  struct ValDouble {
    ValDouble(double v, double vmin=1, double vmax = -1) : current_val(v), default_val(v), val_min(vmin),val_max(vmax) {}
    typedef double valtype;
    double current_val;
    double default_val;
    double val_min;
    double val_max;
    bool isAllowed(double v) const { return val_min>val_max || (v>=val_min&&v<=val_max); }
    TieList ties;
  };
  struct ValInt {
    ValInt(int v, int vmin=1, int vmax = -1) : current_val(v), default_val(v), val_min(vmin),val_max(vmax) {}
    typedef int valtype;
    int current_val;
    int default_val;
    int val_min;
    int val_max;
    bool isAllowed(int v) const { return val_min>val_max || (v>=val_min&&v<=val_max); }
    TieList ties;
  };
  struct ValBool {
    ValBool(int v) : current_val(v), default_val(v) {}
    typedef bool valtype;
    bool current_val;
    bool default_val;
    bool isAllowed(bool) const { return true; }
    TieList ties;
  };
  struct ValStr {
    ValStr(const std::string& v) : current_val(v), default_val(v) {}
    typedef std::string valtype;
    std::string current_val;
    std::string default_val;
    bool isAllowed(const std::string&) const { return true; }
    TieList ties;
  };

  mutable bool m_locked;
  mutable bool m_validating;
  bool m_ignoreRanges;
  bool m_hardExit;
  void fail(const char * message = 0) const {
    std::cout<<std::flush;
    if (message) {
      printf("\nParameters ERROR: %s\n",message);
      std::cout<<std::flush;
    }
    if (m_hardExit)
      exit(1);
    else
      throw std::runtime_error("Parameter failure");
  }

  ParametersBase* m_pb;
  std::map<std::string,ValDouble,fast_str_cmp> m_mapDouble;
  std::map<std::string,ValInt,fast_str_cmp> m_mapInt;
  std::map<std::string,ValBool,fast_str_cmp> m_mapBool;
  std::map<std::string,ValStr,fast_str_cmp> m_mapStr;
  std::set<std::string,fast_str_cmp> m_allnames;
  std::vector<std::pair<std::string,int> > m_addedorder;
  std::set<std::string,fast_str_cmp> m_block_setval;

  void lock() const;

  template<class Tmap, class Tval>
  void mapAdd(const std::string& name, Tmap& themap, const Tval&theval, int itype)
  {
    std::cout<<std::flush;
    if (m_locked)
      fail("Attempt to define parameters on locked object");
    if (name.empty())
      fail("Attempt to add parameter with empty name");
    assert(theval.current_val==theval.default_val);
    if (m_allnames.count(name)>0) {
      printf("\nParameters ERROR: Attempt to add parameter named %s more than once\n",name.c_str());
      fail();
    }
    m_allnames.insert(name);
    assert(themap.find(name)==themap.end());
    m_addedorder.push_back(std::make_pair(name,itype));
    themap.insert(std::make_pair(name,theval));
    if (!theval.isAllowed(theval.default_val)) {
      printf("\nParameters ERROR: Attempt to add parameter named %s with disallowed default value!\n",name.c_str());
      fail();
    }
  }

  template<class Tmap, class Tval>
  void setVal(const std::string& name, Tmap& themap, const Tval&theval)
  {
    std::cout<<std::flush;
    if (!m_block_setval.empty()&&m_block_setval.count(name)>0)
      return;

    if (m_locked) {
      printf("\nParameters ERROR: Attempt to set parameter named %s after values are locked!\n",name.c_str());
      fail();
    }
    auto it = themap.find(name);
    if (it==themap.end()) {
      if (m_allnames.count(name)>0)
        printf("\nParameters ERROR: Attempt to set value of parameter %s to wrong type of value!\n",name.c_str());
      else
        printf("\nParameters ERROR: Attempt to set value of unknown parameter %s!\n",name.c_str());
      fail();
    }
    if (!m_ignoreRanges&&!it->second.isAllowed(theval)) {
      std::cout<<"\nParameters ERROR: Attempt to set parameter "<<name<<" to disallowed value "<<theval<<"!"<<std::endl;
      fail();
    }
    it->second.current_val = theval;
    if ( ! it->second.ties.empty() ) {
      m_block_setval.insert(name);
      for (auto itTie = it->second.ties.begin(); itTie!=it->second.ties.end(); ++itTie)
        itTie->first->setParameter(itTie->second,theval);
      m_block_setval.erase(name);
    }
  }

  template<class Tmap>
  void untie(Tmap& themap)
  {
    std::set<ParametersBase*> tie_holders;
    auto itME = themap.end();
    for (auto itM = themap.begin(); itM!=itME; ++itM) {
      if (itM->second.ties.empty())
        continue;
      auto itE = itM->second.ties.end();
      for (auto it = itM->second.ties.begin(); it!=itE; ++it) {
        if (it->first!=m_pb)
          tie_holders.insert(it->first);
      }
    }
    auto itE = tie_holders.end();
    for (auto it = tie_holders.begin(); it!=itE; ++it) {
      (*it)->m_imp->removeMapTiesTo(m_pb,(*it)->m_imp->m_mapDouble);
      (*it)->m_imp->removeMapTiesTo(m_pb,(*it)->m_imp->m_mapInt);
      (*it)->m_imp->removeMapTiesTo(m_pb,(*it)->m_imp->m_mapBool);
      (*it)->m_imp->removeMapTiesTo(m_pb,(*it)->m_imp->m_mapStr);
    }
  }

  struct RemoveTie {
    RemoveTie(ParametersBase *t) : m_target(t) {}
    bool operator()(const TieList::value_type& e) const { return m_target == e.first;  }
    ParametersBase * m_target;
  };

  template<class Tmap>
  void removeMapTiesTo(ParametersBase* p,Tmap& themap)
  {
    //If only we didn't have to support gcc 4.4, we could do the cleanup in 2 lines:
    //auto l = [p]( const TieList::value_type& e ){ return e.first == p; };
    //for (auto& kv : themap) kv.second.ties.remove_if(l);

    RemoveTie rt(p);
    auto itE = themap.end();
    for (auto it = themap.begin();it!=itE;++it)
      it->second.ties.remove_if(rt);
  }

  template<class Tmap>
  const typename Tmap::value_type::second_type::valtype& mapGetVal(const std::string& name, const Tmap& themap, bool dolock=true) const
  {
    if (dolock&&!m_locked)
      lock();
    std::cout<<std::flush;
    auto it = themap.find(name);
    if (it==themap.end()) {
      if (m_allnames.count(name)>0)
        printf("\nParameters ERROR: Type mismatch when attempting to get value of parameter %s!\n",name.c_str());
      else
        printf("\nParameters ERROR: Attempt to get value of unknown parameter %s!\n",name.c_str());
      fail();
    }
    return it->second.current_val;
  }

};

Utils::ParametersBase::ParametersBase()
  : m_imp(new Imp(this))
{
}

Utils::ParametersBase::~ParametersBase()
{
  delete m_imp;
}

void Utils::ParametersBase::tieParameters(const std::string&name1, const std::string& name2)
{
  tieParameters(name1,this,name2);
}

void Utils::ParametersBase::tieParameters(const std::string&name, ParametersBase * other, const std::string& other_name)
{
  if (other==this&&name==other_name)
    return;//no need to tie a parameter to itself...
  std::cout<<std::flush;
  if (hasParameterDouble(name)) {
    if (!other->hasParameterDouble(other_name)) {
      printf("\nParameters ERROR: tieParameters failed (%s is not a parameterDouble on the other object)!\n",other_name.c_str());
      m_imp->fail();
    }
    m_imp->m_mapDouble.find(name)->second.ties.push_back(std::make_pair(other,other_name));
    other->m_imp->m_mapDouble.find(other_name)->second.ties.push_back(std::make_pair(this,name));
    setParameterDouble(name,other->getParameterDoubleNoLock(other_name));
  } else if (hasParameterInt(name)) {
    if (!other->hasParameterInt(other_name)) {
      printf("\nParameters ERROR: tieParameters failed (%s is not a parameterInt on the other object)!\n",other_name.c_str());
      m_imp->fail();
    }
    m_imp->m_mapInt.find(name)->second.ties.push_back(std::make_pair(other,other_name));
    other->m_imp->m_mapInt.find(other_name)->second.ties.push_back(std::make_pair(this,name));
    setParameterInt(name,other->getParameterIntNoLock(other_name));
  } else if (hasParameterBoolean(name)) {
    if (!other->hasParameterBoolean(other_name)) {
      printf("\nParameters ERROR: tieParameters failed (%s is not a parameterBoolean on the other object)!\n",other_name.c_str());
      m_imp->fail();
    }
    m_imp->m_mapBool.find(name)->second.ties.push_back(std::make_pair(other,other_name));
    other->m_imp->m_mapBool.find(other_name)->second.ties.push_back(std::make_pair(this,name));
    setParameterBoolean(name,other->getParameterBooleanNoLock(other_name));
  } else if (hasParameterString(name)) {
    if (!other->hasParameterString(other_name)) {
      printf("\nParameters ERROR: tieParameters failed (%s is not a parameterString on the other object)!\n",other_name.c_str());
      m_imp->fail();
    }
    m_imp->m_mapStr.find(name)->second.ties.push_back(std::make_pair(other,other_name));
    other->m_imp->m_mapStr.find(other_name)->second.ties.push_back(std::make_pair(this,name));
    setParameterString(name,other->getParameterStringNoLock(other_name));
    std::cout<<std::flush;
  }
}

void Utils::ParametersBase::setParameterDouble(const std::string&name, double value)
{
  m_imp->setVal(name,m_imp->m_mapDouble,value);
}

void Utils::ParametersBase::setParameterInt(const std::string&name, int value)
{
  m_imp->setVal(name,m_imp->m_mapInt,value);
}

void Utils::ParametersBase::setParameterBoolean(const std::string&name, bool value)
{
  m_imp->setVal(name,m_imp->m_mapBool,value);
}

void Utils::ParametersBase::setParameterString(const std::string&name, const std::string& value)
{
  m_imp->setVal(name,m_imp->m_mapStr,value);
}

void Utils::ParametersBase::lock()
{
  m_imp->lock();
}

bool Utils::ParametersBase::isLocked() const
{
  return m_imp->m_locked;
}

void Utils::ParametersBase::Imp::lock() const
{
  if (m_locked)
    return;
  if (m_validating)
    return;
  m_validating=true;
  //We don't lock during validation (in case coupled parameters needs to be
  //updated, e.g. if one has two ways of specifying a value and the other one
  //has to be updated automatically for future reference).
  if (!m_pb->validateParameters())
    fail("Parameter validation failed!");
  m_locked=true;
  m_validating=false;
}

void Utils::ParametersBase::addParameterDouble(const std::string&name, double default_value)
{
  m_imp->mapAdd(name,m_imp->m_mapDouble,Imp::ValDouble(default_value),0);
}

void Utils::ParametersBase::addParameterDouble(const std::string&name, double default_value,double valmin, double valmax)
{
  m_imp->mapAdd(name,m_imp->m_mapDouble,Imp::ValDouble(default_value,valmin,valmax),0);
}

void Utils::ParametersBase::addParameterInt(const std::string&name, int default_value)
{
  m_imp->mapAdd(name,m_imp->m_mapInt,Imp::ValInt(default_value),1);
}

void Utils::ParametersBase::addParameterInt(const std::string&name, int default_value,int valmin, int valmax)
{
  m_imp->mapAdd(name,m_imp->m_mapInt,Imp::ValInt(default_value,valmin,valmax),1);
}

void Utils::ParametersBase::addParameterBoolean(const std::string&name, bool default_value)
{
  m_imp->mapAdd(name,m_imp->m_mapBool,Imp::ValBool(default_value),2);
}

void Utils::ParametersBase::addParameterString(const std::string&name, const std::string& default_value)
{
  m_imp->mapAdd(name,m_imp->m_mapStr,Imp::ValStr(default_value),3);
}

double Utils::ParametersBase::getParameterDouble(const std::string&name) const
{
  return m_imp->mapGetVal(name,m_imp->m_mapDouble);
}

int Utils::ParametersBase::getParameterInt(const std::string&name) const
{
  return m_imp->mapGetVal(name,m_imp->m_mapInt);
}

double Utils::ParametersBase::getParameterBoolean(const std::string&name) const
{
  return m_imp->mapGetVal(name,m_imp->m_mapBool);
}

const std::string& Utils::ParametersBase::getParameterString(const std::string&name) const
{
  return m_imp->mapGetVal(name,m_imp->m_mapStr);
}

double Utils::ParametersBase::getParameterDoubleNoLock(const std::string&name) const
{
  return m_imp->mapGetVal(name,m_imp->m_mapDouble,false);
}

int Utils::ParametersBase::getParameterIntNoLock(const std::string&name) const
{
  return m_imp->mapGetVal(name,m_imp->m_mapInt,false);
}

bool Utils::ParametersBase::getParameterBooleanNoLock(const std::string&name) const
{
  return m_imp->mapGetVal(name,m_imp->m_mapBool,false);
}

const std::string& Utils::ParametersBase::getParameterStringNoLock(const std::string&name) const
{
  return m_imp->mapGetVal(name,m_imp->m_mapStr,false);
}

void Utils::ParametersBase::dumpNoLock(const char* prefix) const
{
  std::cout<<std::flush;
  auto itE = m_imp->m_addedorder.end();
  for (auto it = m_imp->m_addedorder.begin(); it!=itE; ++it) {
    if (it->second==0) {
      printf("%s[dbl] %s = %.14g\n",prefix,it->first.c_str(),m_imp->m_mapDouble.find(it->first)->second.current_val);
    } else if (it->second==1) {
      printf("%s[int] %s = %i\n",prefix,it->first.c_str(),m_imp->m_mapInt.find(it->first)->second.current_val);
    } else if (it->second==2) {
      printf("%s[flg] %s = %s\n",prefix,it->first.c_str(),m_imp->m_mapBool.find(it->first)->second.current_val?"yes":"no");
    } else if (it->second==3) {
      printf("%s[str] %s = \"%s\"\n",prefix,it->first.c_str(),m_imp->m_mapStr.find(it->first)->second.current_val.c_str());
    } else {
      assert(false);//should never happen
    }
  }
  std::cout<<std::flush;
}

void Utils::ParametersBase::dump(const char* prefix) const
{
  if (!isLocked())
    m_imp->lock();
  dumpNoLock(prefix);
}

bool Utils::ParametersBase::hasParameterDouble(const std::string& name) const
{
  return m_imp->m_mapDouble.find(name)!=m_imp->m_mapDouble.end();
}

bool Utils::ParametersBase::hasParameterInt(const std::string& name) const
{
  return m_imp->m_mapInt.find(name)!=m_imp->m_mapInt.end();
}

bool Utils::ParametersBase::hasParameterBoolean(const std::string& name) const
{
  return m_imp->m_mapBool.find(name)!=m_imp->m_mapBool.end();
}

bool Utils::ParametersBase::hasParameterString(const std::string& name) const
{
  return m_imp->m_mapStr.find(name)!=m_imp->m_mapStr.end();
}

void Utils::ParametersBase::getParameterListDouble(std::vector<std::string>&v) const
{
  v.reserve(v.size()+m_imp->m_mapDouble.size());
  auto itE=m_imp->m_mapDouble.end();
  for (auto it=m_imp->m_mapDouble.begin();it!=itE;++it)
    v.push_back(it->first);
}

void Utils::ParametersBase::getParameterListInt(std::vector<std::string>&v) const
{
  v.reserve(v.size()+m_imp->m_mapInt.size());
  auto itE=m_imp->m_mapInt.end();
  for (auto it=m_imp->m_mapInt.begin();it!=itE;++it)
    v.push_back(it->first);
}

void Utils::ParametersBase::getParameterListBoolean(std::vector<std::string>&v) const
{
  v.reserve(v.size()+m_imp->m_mapBool.size());
  auto itE=m_imp->m_mapBool.end();
  for (auto it=m_imp->m_mapBool.begin();it!=itE;++it)
    v.push_back(it->first);
}

void Utils::ParametersBase::getParameterListString(std::vector<std::string>&v) const
{
  v.reserve(v.size()+m_imp->m_mapStr.size());
  auto itE=m_imp->m_mapStr.end();
  for (auto it=m_imp->m_mapStr.begin();it!=itE;++it)
    v.push_back(it->first);
}

void Utils::ParametersBase::noHardExitOnParameterFailure()
{
  m_imp->m_hardExit = false;
}

void Utils::ParametersBase::setIgnoreRanges()
{
  if (m_imp->m_ignoreRanges)
    return;
  std::cout<<std::flush;
  printf("Parameters WARNING: Ignoring range checks on double and int parameters!\n");
  std::cout<<std::flush;
  m_imp->m_ignoreRanges = true;
}

void Utils::ParametersBase::getParameterString_SplitAsVector(const std::string&name, std::vector<std::string>&v) const
{
  v.clear();
  Core::split(v,getParameterString(name),";:,");
}

void Utils::ParametersBase::serialiseParameters(char*& output, unsigned& outputLength) const
{
  m_imp->lock();

  //////////////////////////////////
  //  Figure out length of data:  //
  //////////////////////////////////

  unsigned n = ByteStream::nbytesToWrite<std::uint32_t>();
  //Parameter names and types
  auto itE = m_imp->m_addedorder.end();
  for (auto it = m_imp->m_addedorder.begin(); it!=itE; ++it)
    n += ByteStream::nbytesToWrite(it->first) + ByteStream::nbytesToWrite<std::uint8_t>();
  //parameter values:
  static_assert(sizeof(double)==8);
  n += m_imp->m_mapDouble.size()*ByteStream::nbytesToWrite<double>();
  n += m_imp->m_mapInt.size()*ByteStream::nbytesToWrite<std::int32_t>();
  n += m_imp->m_mapBool.size()*ByteStream::nbytesToWrite<std::uint8_t>();
  auto itPSE = m_imp->m_mapStr.end();
  for (auto itPS = m_imp->m_mapStr.begin(); itPS!=itPSE;++itPS)
    n+= ByteStream::nbytesToWrite(itPS->second.current_val);
  outputLength=n;

  /////////////////////////////////
  //  Allocate and return data:  //
  /////////////////////////////////

  output = new char[n];
  char* data = output;
#ifndef NDEBUG
  auto dataExpectedEnd = output+n;
  for (unsigned i=0;i<n;++i)
    data[i]='%';
#endif
  ByteStream::write(data,(std::uint32_t)m_imp->m_addedorder.size());
  for (auto it = m_imp->m_addedorder.begin(); it!=itE; ++it) {
    ByteStream::write(data,it->first);
    assert(it->second>=0&&it->second<UINT8_MAX);
    ByteStream::write(data,std::uint8_t(it->second));
    if (it->second==0) {
      ByteStream::write(data,m_imp->m_mapDouble.find(it->first)->second.current_val);
    } else if (it->second==1) {
      int val = m_imp->m_mapInt.find(it->first)->second.current_val;
      assert(val>INT32_MIN&&val<INT32_MAX);
      ByteStream::write(data,(std::int32_t)val);
    } else if (it->second==2) {
      ByteStream::write(data,(std::uint8_t)(m_imp->m_mapBool.find(it->first)->second.current_val?1:0));
    } else if (it->second==3) {
      ByteStream::write(data,m_imp->m_mapStr.find(it->first)->second.current_val);
    } else {
      assert(false);//should never happen
    }
  }
  assert(data==dataExpectedEnd);
}

void Utils::ParametersBase::exposeParameter(const std::string& other_name, ParametersBase * other, const std::string& new_name, bool tieOnClash )
{
  std::string n = new_name.empty() ? other_name : new_name;

  if (m_imp->m_allnames.count(n)>0) {
    if (tieOnClash) {
      tieParameters(n, other, other_name);
      return;
    } else {
      std::cout<<std::flush;
      printf("\nParameters ERROR: Attempt to expose parameter whose name clash with an existing parameter: %s\n",n.c_str());
      m_imp->fail();
    }
  }

  //No need to specify value constraints in the addParameterXXX calls below,
  //since the tie to a constrained parameter will be enough.

  if (other->hasParameterDouble(other_name)) {
    addParameterDouble(n,other->getParameterDoubleNoLock(other_name));
  } else if (other->hasParameterInt(other_name)) {
    addParameterInt(n,other->getParameterIntNoLock(other_name));
  } else if (other->hasParameterString(other_name)) {
    addParameterString(n,other->getParameterStringNoLock(other_name));
  } else if (other->hasParameterBoolean(other_name)) {
    addParameterBoolean(n,other->getParameterBooleanNoLock(other_name));
  } else {
    std::cout<<std::flush;
    printf("\nParameters ERROR: Attempting to expose non-existing parameter \"%s\"!\n",other_name.c_str());
    m_imp->fail();
  }
  tieParameters(n,other,other_name);
}

void Utils::ParametersBase::exposeParameters(ParametersBase * other, const std::string& prefix, bool tieOnClash)
{
  ParameterSet ep;
  exposeParameters(other,ep,prefix,tieOnClash);
}

void Utils::ParametersBase::exposeParameters(ParametersBase * other,
                                             const Utils::ParametersBase::ParameterSet& ep,
                                             const std::string& prefix,
                                             bool tieOnClash )
{
  auto itE = other->m_imp->m_addedorder.end();
  for (auto it = other->m_imp->m_addedorder.begin(); it!=itE; ++it) {
    if (ep.count(it->first)>0)
      continue;
    exposeParameter(it->first, other, prefix+it->first, tieOnClash );
  }
}
