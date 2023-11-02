#include "SimpleHists/HistCounts.hh"
#include "labelutils.hh"
#include "floatcompat.hh"
#include <stdexcept>
#include <cstring>//for memcpy

SimpleHists::HistCounts::HistCounts()
  : HistBase()
{
}

SimpleHists::HistCounts::HistCounts(const std::string& title_or_serialised_data)
  : HistBase()
{
  if (!title_or_serialised_data.empty()) {
    if (histTypeOfData(title_or_serialised_data)==0x03)
      {
        perform_deserialisation(title_or_serialised_data);
      }
    else
      {
        //Passed string was title.
        setTitle(title_or_serialised_data);
      }
  }
}

SimpleHists::HistCounts::~HistCounts()
{
}

SimpleHists::HistCounts::Counter SimpleHists::HistCounts::addCounter(const std::string& label,const std::string& display_label)
{
  if (m_counters.size()==100000)
    throw std::runtime_error("HistCounts: Too many counters.");

  if (label.empty()||label.size()>60)
    throw std::runtime_error("HistCounts: labels must be between 1 and 60 characters long");

  if (display_label.size()>60)
    throw std::runtime_error("HistCounts: display label must not be more than 60 characters long");

  std::string dl(display_label.empty()?label:display_label);
  std::string l(label);

  //After lowercasing and converting spaces to underscores, labels must be valid
  //as python identifiers and unique.

  convert_to_lowercase(l);
  convert_spaces_to_underscores(l);

  if (!is_valid_python_identifier(l,false)) {
    printf("HistCounts::addCounter ERROR: Counter created with invalid label \"%s\"\n",label.c_str());
    printf("HistCounts::addCounter        * First character must be a letter.\n");
    printf("HistCounts::addCounter        * Remaining characters must all be letters, digits, spaces or underscores.\n");
    printf("HistCounts::addCounter        * If you wish to include other characters for a nicer display in plots, you can\n");
    printf("HistCounts::addCounter          pass a second parameter to the addCounter call with a label only used for\n");
    printf("HistCounts::addCounter          display purposes.\n");
    throw std::runtime_error("HistCounts:addCounter invalid label");
  }

  auto it = m_label2counter.find(l);
  if (it!=m_label2counter.end()) {
    printf("HistCounts::addCounter ERROR: Counter created with non-unique label \"%s\"\n",label.c_str());
    printf("HistCounts::addCounter Note that labels must be unique, even when lowercased and with underscores instead of strings.\n");
    throw std::runtime_error("HistCounts::addCounter invalid label");
  }
  m_counters.push_back(CounterInternal(l,dl));
  CounterInternal* ci = &(m_counters.back());
  m_label2counter[l] = ci;
  return Counter(ci);
}

const SimpleHists::HistCounts::Counter SimpleHists::HistCounts::getCounter(const std::string& label) const
{
  auto it = m_label2counter.find(label);
  if (it!=m_label2counter.end())
    return Counter(it->second);
  throw std::runtime_error("HistCounts:getCounter unknown counter label");
  //Update: Removed the "smart" stuff below since it encourages inefficient code.
  // //Try with lowercase and underscores.
  // std::string l(label);
  // convert_to_lowercase(l);
  // convert_spaces_to_underscores(l);
  // it = m_label2counter.find(l);
  // if (it!=m_label2counter.end())
  //   return Counter(it->second);
  // //Error!
  // printf("HistCounts::getCounter ERROR: Unknown counter label requested: \"%s\"\n",label.c_str());
  // if (m_counters.size()<100) {
  //   printf("HistCounts::getCounter    Available counters are:\n");
  //   for (auto itC=m_counters.begin();itC!=m_counters.end();++itC)
  //     printf("HistCounts::getCounter        \"%s\"\n",itC->m_label.c_str());
  // }
}

void SimpleHists::HistCounts::Counter::setDisplayLabel(const std::string& dl)
{
  if (dl.size()>60)
    throw std::runtime_error("HistCounts: display label must not be more than 60 characters long");
  m_counter->m_displayLabel = dl;
}

void SimpleHists::HistCounts::Counter::setComment(const std::string& c)
{
  if (c.size()>8192)
    throw std::runtime_error("HistCounts: counter comments must not be more than 8192 characters long");
  m_counter->m_comment = c;
}

SimpleHists::HistCounts::Counter SimpleHists::HistCounts::getCounter(const std::string& label)
{
  return const_cast<const HistCounts*>(this)->getCounter(label);
}

bool SimpleHists::HistCounts::empty() const
{
  auto it = m_counters.begin();
  auto itE = m_counters.end();
  for (;it!=itE;++it)
    if (it->m_content)
      return false;
  return true;
}

double SimpleHists::HistCounts::getIntegral() const
{
  auto it = m_counters.begin();
  auto itE = m_counters.end();
  double ig = 0.0;
  for (;it!=itE;++it)
    ig += it->m_content;
  return ig;
}

void SimpleHists::HistCounts::reset()
{
  auto it = m_counters.begin();
  auto itE = m_counters.end();
  for (;it!=itE;++it)
    it->m_content = it->m_errorsq = 0.0;
}

SimpleHists::HistBase* SimpleHists::HistCounts::clone() const
{
  HistCounts * h = new HistCounts(getTitle());
  h->setXLabel(getXLabel());
  h->setYLabel(getYLabel());
  h->setComment(getComment());
  auto it = m_counters.begin();
  auto itE = m_counters.end();
  for (;it!=itE;++it) {
    h->m_counters.push_back(*it);
    h->m_label2counter[it->m_label] = &(h->m_counters.back());
  }
  return h;
}

bool SimpleHists::HistCounts::isSimilar(SimpleHists::HistBase const* obase) const
{
  if (!HistBase::isSimilar(obase))
    return false;
  const HistCounts * o = dynamic_cast<const HistCounts*>(obase);
  assert(o);//HistBase already checked histType
  if (m_counters.size()!=o->m_counters.size())
    return false;
  auto it = m_counters.begin();
  auto ito = o->m_counters.begin();
  auto itE = m_counters.end();
  for (;it!=itE;++it,++ito) {
    if (!floatCompatible(it->m_content,ito->m_content))
      return false;
    if (!floatCompatible(it->m_errorsq,ito->m_errorsq))
      return false;
    if (it->m_label!=ito->m_label)
      return false;
    if (it->m_displayLabel!=ito->m_displayLabel)
      return false;
    if (it->m_comment!=ito->m_comment)
      return false;
  }
  assert(ito==o->m_counters.end());
  return true;
}

void SimpleHists::HistCounts::scale(double a)
{
  if (a==1.0)
    return;
  if (a<=0)
    throw std::runtime_error("HistCounts: scale factor must be >0");
  auto it = m_counters.begin();
  auto itE = m_counters.end();
  const double a2(a*a);
  for (;it!=itE;++it) {
    it->m_content *= a;
    it->m_errorsq *= a2;
  }
}

void SimpleHists::HistCounts::dump(bool contents,const std::string& prefix) const
{
  const char * p = prefix.c_str();
  printf("%sHistCounts():\n",p);
  dumpBase(p);
  double ig = getIntegral();
  printf("%s  integral  : %g\n",p,ig);
  if (contents) {
    auto it = m_counters.begin();
    auto itE = m_counters.end();
    for (;it!=itE;++it) {
      Counter c(const_cast<CounterInternal*>(&(*it)));
      printf("%s  counter[%s] = %g +- %g\n",p,c.getDisplayLabel().c_str(),c.getValue(),c.getError());
      if (!c.getComment().empty())
        printf("%s    => %s\n",p,c.getComment().c_str());
    }
  }
}

bool SimpleHists::HistCounts::mergeCompatible(SimpleHists::HistBase const*obase) const
{
  if (!HistBase::mergeCompatible(obase))
    return false;
  const HistCounts* o = dynamic_cast<const HistCounts*>(obase);
  if (!o)
    return false;
  if (m_counters.size()!=o->m_counters.size())
    return false;
  auto it = m_counters.begin();
  auto ito = o->m_counters.begin();
  auto itE = m_counters.end();
  for (;it!=itE;++it,++ito) {
    if (it->m_label!=ito->m_label)
      return false;
    if (it->m_displayLabel!=ito->m_displayLabel)
      return false;
    if (it->m_comment!=ito->m_comment)
      return false;
  }
  assert(ito==o->m_counters.end());
  return true;//succes!
}

void SimpleHists::HistCounts::merge(SimpleHists::HistBase const*obase)
{
  assert(obase);
  if (!mergeCompatible(obase))
    throw std::runtime_error("Attempting to merge incompatible counts histograms");
  const HistCounts * o = dynamic_cast<const HistCounts*>(obase);
  assert(o);

  auto it = m_counters.begin();
  auto ito = o->m_counters.begin();
  auto itE = m_counters.end();
  for (;it!=itE;++it,++ito) {
    it->m_content += ito->m_content;
    it->m_errorsq += ito->m_errorsq;
  }
  assert(ito==o->m_counters.end());
}

void SimpleHists::HistCounts::serialise(std::string& buf) const
{
  //Layout:
  // [HistBase stuff] + 0 char + [each counter in turn]

  unsigned nbase = serialisedBaseSize();

  //size needed for contents, except actual label char arrays, remember
  //|labels|<=60 so the size fits in a char and |comments|<65353 so size fits in short:
  unsigned n = 1 + m_counters.size() * (2*sizeof(double)+4*sizeof(char));

  //Run through once to figure out size in buf.
  auto itE = m_counters.end();
  for (auto it = m_counters.begin();it!=itE;++it) {
    n+=it->m_label.size();
    n+=it->m_displayLabel.size();
    n+=it->m_comment.size();
  }

  //make space:
  buf.resize(n+nbase);//a bit wasteful initialisation

  //HistBase stuff:
  serialiseBaseToBuffer(&(buf[0]));

  //0 char:
  buf[nbase] = 0;

  //Our contents (lots of memcpy calls is a bit wasteful but safe and we don't expect too many bins):
  unsigned offset(nbase+1);
  for (auto it = m_counters.begin();it!=itE;++it) {
    std::memcpy(&(buf[offset]),&(it->m_content),sizeof(it->m_content)); offset += sizeof(it->m_content);
    std::memcpy(&(buf[offset]),&(it->m_errorsq),sizeof(it->m_errorsq)); offset += sizeof(it->m_errorsq);
    buf[offset] = (char)(it->m_label.size()); offset += 1;
    buf[offset] = (char)(it->m_displayLabel.size()); offset += 1;
    buf[offset] = (std::uint16_t)(it->m_comment.size()); offset += 2;
    std::memcpy(&(buf[offset]),&(it->m_label[0]),it->m_label.size()); offset += it->m_label.size();
    std::memcpy(&(buf[offset]),&(it->m_displayLabel[0]),it->m_displayLabel.size()); offset += it->m_displayLabel.size();
    std::memcpy(&(buf[offset]),&(it->m_comment[0]),it->m_comment.size()); offset += it->m_comment.size();
  }
  assert(buf.size()==offset&&offset==n+nbase);
}

void SimpleHists::HistCounts::perform_deserialisation(const std::string& buf)
{
  unsigned offset;
  char version;
  deserialiseBase(buf,offset,version);

  //Version should have already been checked in deserialiseBase. The HistCounts
  //treatment is the same for versions 0x01 and 0x02.
  assert(version==0x01||version==0x02);

  if (buf[offset]!=0)
    throw std::runtime_error("HistCounts deserialisation failure - data not in correct format!");
  offset+=1;

  CounterInternal tmp;
  unsigned label_size, displaylabel_size, comment_size;
  while (offset<buf.size()) {
    std::memcpy(&tmp.m_content,&(buf[offset]),sizeof(tmp.m_content)); offset += sizeof(tmp.m_content);
    std::memcpy(&tmp.m_errorsq,&(buf[offset]),sizeof(tmp.m_errorsq)); offset += sizeof(tmp.m_errorsq);
    label_size = buf[offset]; offset+=1;
    displaylabel_size = buf[offset]; offset+=1;
    comment_size = *((std::uint16_t*)&buf[offset]); offset+=2;
    tmp.m_label.assign(&buf[offset],label_size); offset+=label_size;
    tmp.m_displayLabel.assign(&buf[offset],displaylabel_size); offset+=displaylabel_size;
    tmp.m_comment.assign(&buf[offset],comment_size); offset+=comment_size;
    m_counters.push_back(tmp);
    m_label2counter[tmp.m_label] = &(m_counters.back());
  }
}

double SimpleHists::HistCounts::getMaxContent() const
{
  double mc(0.0);
  //Used to have this (but decided that empty HistCounts are still valid in a way):
  //if (m_counters.empty())
  //    throw std::runtime_error("HistCounts: getMaxContent called before counters were added.");
  auto it = m_counters.begin();
  auto itE = m_counters.end();
  for (;it!=itE;++it)
    if (mc<it->m_content)
      mc=it->m_content;
  return mc;
}

void SimpleHists::HistCounts::setErrorsByContent()
{
  auto it = m_counters.begin();
  auto itE = m_counters.end();
  for (;it!=itE;++it)
    it->m_errorsq = it->m_content;
}

void SimpleHists::HistCounts::getCounters(std::list<Counter>&l)
{
  auto it = m_counters.begin();
  auto itE = m_counters.end();
  for (;it!=itE;++it)
    l.push_back(&(*it));
}


void SimpleHists::HistCounts::sortByLabels()
{
  m_counters.sort( []( const CounterInternal &a, const CounterInternal &b )
                   { return a.m_label < b.m_label; } );
}

void SimpleHists::HistCounts::sortByDisplayLabels()
{
  m_counters.sort( []( const CounterInternal &a, const CounterInternal &b )
                   { return a.m_displayLabel < b.m_displayLabel; } );
}

