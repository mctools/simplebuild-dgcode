#include "SimpleHists/HistBase.hh"
#include <cstring>
#include <stdexcept>

SimpleHists::HistBase::~HistBase()
{
}

//Format of serialised data:
// ----- Hist Base -----
//1 byte: format version (x01)
//1 byte: histogram type (x01=>Hist1D, x02=>Hist2D, x03=>HistCounts)
//2 bytes: title length (NT)
//NT bytes: title data
//2 bytes: xLabel length (NX)
//NX bytes: xLabel data
//2 bytes: yLabel length (NY)
//NY bytes: yLabel data
//2 bytes: comment length (NC)
//NC bytes: comment data
// ----- Derived class -----
//<specific format up to derived class>

unsigned SimpleHists::HistBase::serialisedBaseSize() const
{
  return 10 + m_title.size()+m_xLabel.size()+m_yLabel.size()+m_comment.size();
}

void SimpleHists::HistBase::serialiseBaseToBuffer(char* buf) const
{
  assert(m_title.size()<65535);
  assert(m_xLabel.size()<65535);
  assert(m_yLabel.size()<65535);
  assert(m_comment.size()<65535);

  //write version:
  *(buf++) = 0x02;
  //write hist type:
  unsigned ht = histType();
  assert(ht<=255);
  *(buf++) = static_cast<char>(ht);
  //write title:
  *(reinterpret_cast<std::uint16_t*>(buf)) = static_cast<std::uint16_t>(m_title.size());
  buf+=2;
  std::memcpy(buf,&(m_title[0]),m_title.size());
  buf+=m_title.size();
  //write xLabel:
  *(reinterpret_cast<std::uint16_t*>(buf)) = static_cast<std::uint16_t>(m_xLabel.size());
  buf+=2;
  std::memcpy(buf,&(m_xLabel[0]),m_xLabel.size());
  buf+=m_xLabel.size();
  //write yLabel:
  *(reinterpret_cast<std::uint16_t*>(buf)) = static_cast<std::uint16_t>(m_yLabel.size());
  buf+=2;
  std::memcpy(buf,&(m_yLabel[0]),m_yLabel.size());
  buf+=m_yLabel.size();
  //write comment:
  *(reinterpret_cast<std::uint16_t*>(buf)) = static_cast<std::uint16_t>(m_comment.size());
  buf+=2;
  std::memcpy(buf,&(m_comment[0]),m_comment.size());
  //buf+=m_comment.size(); <-- no effect, so commented
}

void SimpleHists::HistBase::deserialiseBase(const std::string& s,unsigned& offset, char& version)
{
  offset=0;
  version = s[offset++];
  if (version!=0x01&&version!=0x02)
    throw std::runtime_error("Histogram deserialisation failed! (version mismatch)");
  if (s[offset++]!=static_cast<char>(histType()))
    throw std::runtime_error("Histogram deserialisation failed! (histogram type mismatch)");

  std::uint16_t n = *(reinterpret_cast<const std::uint16_t*>(&s[offset]));
  offset += 2;
  if (n)
    m_title.assign(&(s[offset]),n);
  else
    m_title.clear();
  offset += n;

  n = *(reinterpret_cast<const std::uint16_t*>(&s[offset]));
  offset += 2;
  if (n)
    m_xLabel.assign(&(s[offset]),n);
  else
    m_xLabel.clear();
  offset += n;

  n = *(reinterpret_cast<const std::uint16_t*>(&s[offset]));
  offset += 2;
  if (n)
    m_yLabel.assign(&(s[offset]),n);
  else
    m_yLabel.clear();
  offset += n;

  n = *(reinterpret_cast<const std::uint16_t*>(&s[offset]));
  offset += 2;
  if (n)
    m_comment.assign(&(s[offset]),n);
  else
    m_comment.clear();
  offset += n;

  if (offset>s.size())
    throw std::runtime_error("Histogram deserialisation failed! (data size error)");
}


char SimpleHists::histTypeOfData(const std::string& serialised_data)
{
  //Important that at least one of version and histtype is <= 0x08, because
  //those chars should never appear in strings of normal text (0x09 is TAB) =>
  //making our detection here and in e.g. the constructor of HistCounts() much
  //less fragile.

  if (serialised_data.size()<10||(serialised_data[0]!=0x01&&serialised_data[0]!=0x02))
    return 0;
  char ht = serialised_data[1];
  if (ht==0x01||ht==0x02||ht==0x03)
    return ht;
  return 0;
}

#include "SimpleHists/Hist1D.hh"
#include "SimpleHists/Hist2D.hh"
#include "SimpleHists/HistCounts.hh"

SimpleHists::HistBase * SimpleHists::deserialise(const std::string& serialised_data)
{
  char ht = histTypeOfData(serialised_data);
  if (ht==0x01)
    return new Hist1D(serialised_data);
  if (ht==0x02)
    return new Hist2D(serialised_data);
  if (ht==0x03)
    return new HistCounts(serialised_data);
  std::runtime_error("SimpleHists::deserialise bad input data");
  return 0;
}

bool SimpleHists::HistBase::mergeCompatible(const HistBase*o) const
{
  if (histType()!=o->histType()) return false;
  if (getTitle()!=o->getTitle()) return false;
  if (getXLabel()!=o->getXLabel()) return false;
  if (getYLabel()!=o->getYLabel()) return false;
  if (getComment()!=o->getComment()) return false;
  return true;//base ok, derived classes implements additional tests for bins
}

bool SimpleHists::HistBase::isSimilar(const HistBase*o) const
{
  if (histType()!=o->histType()) return false;
  if (getTitle()!=o->getTitle()) return false;
  if (getXLabel()!=o->getXLabel()) return false;
  if (getYLabel()!=o->getYLabel()) return false;
  if (getComment()!=o->getComment()) return false;
  return true;
}

void SimpleHists::HistBase::norm()
{
  double ig = getIntegral();
  if (ig>0)
    scale(1.0/ig);
}

void SimpleHists::HistBase::dumpBase(const char *prefix) const
{
  printf("%s  title     : %s\n",prefix,getTitle().empty() ? "<none>" : getTitle().c_str());
  printf("%s  xLabel    : %s\n",prefix,getXLabel().empty() ? "<none>" : getXLabel().c_str());
  printf("%s  yLabel    : %s\n",prefix,getYLabel().empty() ? "<none>" : getYLabel().c_str());
  printf("%s  comment    : %s\n",prefix,getComment().empty() ? "<none>" : getComment().c_str());
}
