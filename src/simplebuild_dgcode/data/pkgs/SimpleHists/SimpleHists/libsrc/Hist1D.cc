#include "SimpleHists/Hist1D.hh"
#include "hist_stats.hh"
#include "floatcompat.hh"
#include <stdexcept>
#include <cstring>//for memset
#include <algorithm>//for max_element

SimpleHists::Hist1D::~Hist1D()
{
  delete[] m_content;
  delete[] m_errors;
}

void SimpleHists::Hist1D::dump(bool contents,const std::string& prefix) const
{
  const char * p = prefix.c_str();
  printf("%sHist1D(nbins=%i,xmin=%g,xmax=%g):\n",p,getNBins(),getXMin(),getXMax());
  dumpBase(p);
  printf("%s  integral  : %g\n",p,getIntegral());
  if (!empty()) {
    printf("%s  mean      : %g\n",p,getMean());
    printf("%s  rms       : %g\n",p,getRMS());
    int medianbin = getPercentileBin(0.5);
    assert(-1<=medianbin&&medianbin<=(int)m_data.nbins);
    if (medianbin==-1) printf("%s  median    : underflow\n",p);
    else if (medianbin==(int)m_data.nbins) printf("%s  median    : overflow\n",p);
    else printf("%s  median    : %g +- %g\n",p,getBinCenter(medianbin),0.5*getBinWidth());
    printf("%s  minfilled : %g\n",p,getMinFilled());
    printf("%s  maxfilled : %g\n",p,getMaxFilled());
  } else {
    printf("%s  mean      : <n/a>\n",p);
    printf("%s  rms       : <n/a>\n",p);
    printf("%s  median    : <n/a>\n",p);
    printf("%s  minfilled : <n/a>\n",p);
    printf("%s  maxfilled : <n/a>\n",p);
  }
  printf("%s  underflow : %g\n",p,getUnderflow());
  printf("%s  overflow  : %g\n",p,getOverflow());
  if (contents) {
    for (unsigned ibin = 0; ibin < getNBins(); ++ibin)
      printf("%s  content[ibin=%i] : %g +- %g\n",p,ibin,getBinContent(ibin),getBinError(ibin));
  }
}

SimpleHists::Hist1D::Hist1D(const std::string& serialised_data)
  : HistBase()
{
  unsigned offset;
  char version;
  deserialiseBase(serialised_data,offset,version);
  //Version should have already been checked in deserialiseBase. The Hist1D
  //treatment is the same for versions 0x01 and 0x02.
  assert(version==0x01||version==0x02);

  std::memcpy(&(m_serialdata[0]),&(serialised_data[offset]),sizeof(m_serialdata));
  offset += sizeof(m_serialdata);

  int32_t raw_nbins = (int32_t)m_data.nbins;

  if (raw_nbins<0)
    m_data.nbins = (std::uint32_t)(-raw_nbins);

  if (raw_nbins>0 && offset+m_data.nbins*sizeof(double)!=serialised_data.size())
    throw std::runtime_error("Hist1D: Histogram deserialisation failed! (data size error [2])");

  m_content = new double[m_data.nbins];
  std::memcpy(reinterpret_cast<char*>(m_content),&(serialised_data[offset]),m_data.nbins*sizeof(double));
  offset += m_data.nbins*sizeof(double);

  m_errors = 0;//only on demand

  if (raw_nbins<0) {
    if (offset+m_data.nbins*sizeof(double)!=serialised_data.size())
    throw std::runtime_error("Hist1D: Histogram deserialisation failed! (data size error [3])");
    m_errors = new double[m_data.nbins];
    std::memcpy(reinterpret_cast<char*>(m_errors),&(serialised_data[offset]),m_data.nbins*sizeof(double));
  }

  //setup non-persistent vars:
  recalcNonPersistentVars();
}

void SimpleHists::Hist1D::serialise(std::string& buf) const
{
  unsigned larray = m_data.nbins*sizeof(double);
  unsigned nbase = serialisedBaseSize();
  unsigned n = larray*(m_errors?2:1)+sizeof(m_serialdata);

  buf.resize(n+nbase);//a bit wasteful initialisation

  std::uint32_t nbins = m_data.nbins;

  //trick: since nbins<=5e8, we abuse the sign of nbins to indicate whether or
  //not errors are present.
  if (m_errors)
    *reinterpret_cast<int32_t*>(&(const_cast<Hist1D*>(this)->m_data.nbins)) = -((int32_t)(nbins));

  serialiseBaseToBuffer(&(buf[0]));
  std::memcpy(&(buf[nbase]),&(m_serialdata[0]),sizeof(m_serialdata));
  std::memcpy(&(buf[nbase+sizeof(m_serialdata)]),reinterpret_cast<const char*>(m_content),larray);
  if (m_errors)
    std::memcpy(&(buf[nbase+sizeof(m_serialdata)+larray]),reinterpret_cast<const char*>(m_errors),larray);

  if (m_errors)
    const_cast<Hist1D*>(this)->m_data.nbins = nbins;//undo trick
}

void SimpleHists::Hist1D::recalcNonPersistentVars()
{
  m_delta = (m_data.xmax-m_data.xmin)/m_data.nbins;
  m_invDelta = 1.0/m_delta;
}


double SimpleHists::Hist1D::getMinFilled() const
{
  if (empty())
    throw std::runtime_error("Hist1D: min filled not well defined in empty histograms");
  return m_data.minfilled;
}

double SimpleHists::Hist1D::getMaxFilled() const
{
  if (empty())
    throw std::runtime_error("Hist1D: max filled not well defined in empty histograms");
  return m_data.maxfilled;
}


double SimpleHists::Hist1D::getMean() const
{
  return calc_mean(m_data.sumW,m_data.sumWX);
}

double SimpleHists::Hist1D::getRMS() const
{
#if SIMPLEHISTS_ROOT_STYLE_RMS
  return calc_rms(m_data.sumW,m_data.sumWX,m_data.rmsstate);
#else
  return calc_rms(m_data.sumW,m_data.rmsstate);
#endif
}

double SimpleHists::Hist1D::getRMSSquared() const
{
#if SIMPLEHISTS_ROOT_STYLE_RMS
  return calc_rms2(m_data.sumW,m_data.sumWX,m_data.rmsstate);
#else
  return calc_rms2(m_data.sumW,m_data.rmsstate);
#endif
}

void SimpleHists::Hist1D::init(unsigned nbins, double xmin, double xmax)
{
  //Sanity check input data:
  if (!(nbins<=250000000)) throw std::runtime_error("Hist1D: Too many bins in histogram!");
  if (!nbins) throw std::runtime_error("Hist1D: Can not book histogram with zero bins");
  if (xmin!=xmin) throw std::runtime_error("Hist1D: Lower bin range is nan (not-a-number)");
  if (xmax!=xmax) throw std::runtime_error("Hist1D: Upper bin range is nan (not-a-number)");
  if (xmax<=xmin) throw std::runtime_error("Hist1D: Upper bin range must be greater than lower bin range");

  //metadata:
  std::memset(m_serialdata,0,sizeof(m_serialdata));
  m_data.nbins = nbins;
  m_data.xmin = xmin;
  m_data.xmax = xmax;
  m_data.maxfilled = -1;//use max<min to indicate no fills yet
  m_data.minfilled = 1;

  //content
  m_content = new double[nbins];
  std::memset(m_content,0,sizeof(double)*nbins);

  m_errors = 0;//only on demand

  //non-persistent metadata
  recalcNonPersistentVars();
}

bool SimpleHists::Hist1D::mergeCompatible(const HistBase*o) const
{
  if (!HistBase::mergeCompatible(o))
    return false;
  const Hist1D* other_1d = dynamic_cast<const Hist1D*>(o);
  if (!other_1d)
    return false;
  if (getNBins()!=other_1d->getNBins()) return false;
  if (getXMin()!=other_1d->getXMin()) return false;
  if (getXMax()!=other_1d->getXMax()) return false;
  return true;//succes!
}

void SimpleHists::Hist1D::merge(const HistBase*obase)
{
  assert(obase);
  if (!mergeCompatible(obase))
    throw std::runtime_error("Attempting to merge incompatible 1D histograms");

  const Hist1D * o = dynamic_cast<const Hist1D*>(obase);
  assert(o);

  //statistics:
  merge_stats(m_data.sumW, m_data.sumWX, m_data.rmsstate,
              o->m_data.sumW, o->m_data.sumWX, o->m_data.rmsstate);

  merge_maxmin(m_data.minfilled, m_data.maxfilled,
               o->m_data.minfilled, o->m_data.maxfilled);

  m_data.underflow += o->m_data.underflow;
  m_data.overflow += o->m_data.overflow;

  //contents:
  {
    double * it = m_content;
    double * itE = m_content + m_data.nbins;
    double * itO = o->m_content;
    while (it!=itE)
      *(it++) += *(itO++);
    assert((itO - (o->m_content)) == (int)o->m_data.nbins);
  }

  //errors:
  if (m_errors||o->m_errors) {
    if (!m_errors)
      initErrors();
    double * it = m_errors;
    double * itE = m_errors + m_data.nbins;
    double * itO(o->m_errors?o->m_errors:o->m_content);
    while (it!=itE)
      *(it++) += *(itO++);
  }
}

void SimpleHists::Hist1D::fillN(unsigned long N, double val)
{
  if (!N)
    return;
  int ibin = valueToBin(val);
  update_maxmin_filled(m_data.minfilled,m_data.maxfilled,val);
  update_stats_on_fillN(m_data.sumW,m_data.sumWX,m_data.rmsstate,N,val);
  if (ibin<0) {
    m_data.underflow += N;
  } else if (ibin>=static_cast<int>(m_data.nbins)) {
    m_data.overflow += N;
  } else {
    m_content[ibin] += N;
    if (m_errors)
      m_errors[ibin] += N;
  }
}

void SimpleHists::Hist1D::fill(double val)
{
  int ibin = valueToBin(val);
  update_maxmin_filled(m_data.minfilled,m_data.maxfilled,val);
  update_stats_on_fill(m_data.sumW,m_data.sumWX,m_data.rmsstate,val);

  if (ibin<0) {
    ++(m_data.underflow);
  } else if (ibin>=static_cast<int>(m_data.nbins)) {
    ++(m_data.overflow);
  } else {
    ++(m_content[ibin]);
    if (m_errors)
      ++(m_errors[ibin]);
  }
}

void SimpleHists::Hist1D::fill(double val, double weight)
{
  if (weight==1) {
    fill(val);
    return;
  }
  assert(!(weight!=weight)&&"Hist1D ERROR: NAN in input weight!");
  if (weight<=0) {
    if (weight==0)
      return;
    throw std::runtime_error("Hist1D: Fill with negative weights gives ill-defined statistics");
  }
  if (!m_errors)//This is the first fill with weight!=1
    initErrors();

  int ibin = valueToBin(val);
  update_maxmin_filled(m_data.minfilled,m_data.maxfilled,val);
  update_stats_on_fill(m_data.sumW,m_data.sumWX,m_data.rmsstate,val,weight);
  if (ibin<0) {
    m_data.underflow += weight;
  } else if (ibin>=static_cast<int>(m_data.nbins)) {
    m_data.overflow += weight;
  } else {
    m_content[ibin] += weight;
    m_errors[ibin] += weight*weight;
  }
}

void SimpleHists::Hist1D::fillMany(const double* vals, unsigned n)
{
  const double *v(vals);
  const double *vE(vals+n);
  for(;v!=vE;++v)
    fill(*v);
}

void SimpleHists::Hist1D::fillMany(const double* vals, const double* weights, unsigned n)
{
  const double *v(vals);
  const double *vE(vals+n);
  const double *w(weights);
  for(;v!=vE;++v,++w)
    fill(*v,*w);
}

void SimpleHists::Hist1D::initErrors()
{
  assert(!m_errors);
  m_errors = new double[m_data.nbins];
  double *v(m_content), *vE(m_content+m_data.nbins);
  double *e(m_errors);
  for(;v!=vE;++v,++e)
    *e = *v;
}

void SimpleHists::Hist1D::setErrorsByContent()
{
  delete[] m_errors;
  m_errors = nullptr;
}

double SimpleHists::Hist1D::getMaxContent() const
{
  return *std::max_element(m_content,m_content+m_data.nbins);
}

void SimpleHists::Hist1D::scale(double a)
{
  if (a==1.0)
    return;
  if (a<=0)
    throw std::runtime_error("Hist1D: scale factor must be >0");
  if (!m_errors)
    initErrors();
  m_data.sumW *= a;
  m_data.sumWX *= a;
  m_data.rmsstate *= a;
  m_data.underflow *= a;
  m_data.overflow *= a;
  //max/minfilled are not affected when scaled
  double *v(m_content), *vE(m_content+m_data.nbins);
  for(;v!=vE;++v)
    *v *= a;
  const double a2(a*a);
  double *e(m_errors), *eE(m_errors+m_data.nbins);
  for(;e!=eE;++e)
    *e *= a2;
}

bool SimpleHists::Hist1D::isSimilar(const HistBase* obase) const
{
  if (!HistBase::isSimilar(obase))
    return false;
  //non-float metadata first:
  const Hist1D * o = dynamic_cast<const Hist1D*>(obase);
  assert(o);//HistBase already checked histType
  if (m_data.nbins!=o->m_data.nbins) return false;
  if (bool(m_errors)!=bool(o->m_errors)) return false;
  //float metadata:
  if (!floatCompatible(m_data.xmin,o->m_data.xmin)) return false;
  if (!floatCompatible(m_data.xmax,o->m_data.xmax)) return false;
  if (!floatCompatible(m_data.sumW,o->m_data.sumW)) return false;
  if (!floatCompatible(m_data.sumWX,o->m_data.sumWX)) return false;
  if (!floatCompatible(m_data.rmsstate,o->m_data.rmsstate)) return false;
  if (!floatCompatible(m_data.underflow,o->m_data.underflow)) return false;
  if (!floatCompatible(m_data.overflow,o->m_data.overflow)) return false;
  if (!floatCompatible(m_data.minfilled,o->m_data.minfilled)) return false;
  if (!floatCompatible(m_data.maxfilled,o->m_data.maxfilled)) return false;

  //contents and errors:
  {
    const double *it(m_content);
    const double *itE(m_content+m_data.nbins);
    const double *ito(o->m_content);
    for (;it!=itE;++it,++ito) {
      if (!floatCompatible(*it,*ito))
        return false;
    }
  }
  if (m_errors) {
    const double *it(m_errors);
    const double *itE(m_errors+m_data.nbins);
    const double *ito(o->m_errors);
    for (;it!=itE;++it,++ito) {
      if (!floatCompatible(*it,*ito)) {
        printf("incompat error! %g versus %g\n",sqrt(*it),sqrt(*ito));
        return false;
      }
    }
  }
  return true;
}

SimpleHists::HistBase* SimpleHists::Hist1D::clone() const
{
  Hist1D * h = new Hist1D(getTitle(),m_data.nbins,m_data.xmin,m_data.xmax);
  h->setXLabel(getXLabel());
  h->setYLabel(getYLabel());
  h->setComment(getComment());

  std::memcpy(h->m_serialdata,m_serialdata,sizeof(m_serialdata));

  //non-persistent vars:
  h->m_invDelta = m_invDelta;
  h->m_delta = m_delta;

  std::memcpy(h->m_content,m_content,m_data.nbins*sizeof(double));

  if (m_errors) {
    assert(!h->m_errors);
    h->m_errors = new double[m_data.nbins];
    std::memcpy(h->m_errors,m_errors,m_data.nbins*sizeof(double));
  }

  return h;
}

void SimpleHists::Hist1D::reset()
{
  delete[] m_content;
  delete[] m_errors;
  init(m_data.nbins,m_data.xmin,m_data.xmax);
}

void SimpleHists::Hist1D::resetAndRebin(unsigned nbins, double xmin, double xmax)
{
  delete[] m_content;
  delete[] m_errors;
  init(nbins, xmin, xmax);
}

void SimpleHists::Hist1D::rebin(unsigned new_nbins)
{
  if (m_data.nbins==new_nbins)
    return;

  if (!canRebin(new_nbins))
    throw std::runtime_error("Hist1D: invalid rebinning (new nbins must be divisor in old nbins)");

  const int rebin_fact = m_data.nbins / new_nbins;
  double * new_content = new double[new_nbins];
  std::memset(new_content,0,new_nbins*sizeof(double));
  double * itOld = m_content;
  double * itOldE = m_content+m_data.nbins;
  double * itNew = new_content - 1;
  int i=-1;
  for (;itOld!=itOldE;++itOld) {
    if ( ++i % rebin_fact == 0 )
      ++itNew;
    *itNew += *itOld;
  }
  delete[] m_content;
  m_content = new_content;

  if (m_errors) {
    double * new_errors = new double[new_nbins];
    std::memset(new_errors,0,new_nbins*sizeof(double));
    itOld = m_errors;
    itOldE = m_errors+m_data.nbins;
    itNew = new_errors - 1;
    i=-1;
    for (;itOld!=itOldE;++itOld) {
      if ( ++i % rebin_fact == 0 )
        ++itNew;
      *itNew += *itOld;
    }
    assert(itNew+1==new_errors+new_nbins);
    delete[] m_errors;
    m_errors = new_errors;
  }

  m_data.nbins = new_nbins;
  recalcNonPersistentVars();
}

int SimpleHists::Hist1D::getPercentileBin(double percentile) const
{
  if (percentile<=0.0||percentile>=1.0) {
    throw std::runtime_error("Hist1D: getPercentileBin requires 0<percentile<1");
  }
  double ig = getIntegral();
  if (!ig)
    throw std::runtime_error("Hist1D: getPercentileBin undefined on empty histograms");
  assert(ig>0);
  double target = percentile*ig;
  double sum(0.0);
  sum += m_data.underflow;
  if (sum >= target)
    return -1;//requested percentile is in the underflow bin

  double *v(m_content), *vE(m_content+m_data.nbins);
  for(;v!=vE;++v) {
    sum += *v;
    if (sum >= target)
      return v-m_content;
  }

  return m_data.nbins;//requested percentile is in the overflow bin
}

double SimpleHists::Hist1D::getBinSum(int bin1, int bin2)
{
  if (bin1>bin2||bin1<-1||bin2>(int)m_data.nbins)
    throw std::runtime_error("Hist1D: integrate requires -1 <= bin1 <= bin2 <= nbins");

  //Special cases completely excluding internal bins:
  if (bin1==(int)m_data.nbins)
    return m_data.overflow;
  if (bin2==-1)
    return m_data.overflow;

  //prepare for integration
  double sum(0.0);
  double *v(m_content+bin1), *vE(m_content+bin2+1);

  if (bin1==-1)
    {
      ++v;
      assert(v==m_content);
      sum += m_data.underflow;
    }
  if (bin2==(int)m_data.nbins)
    {
      --vE;
      assert(vE==m_content+m_data.nbins);
      sum += m_data.overflow;
    }

  for(;v!=vE;++v)
    {
      sum += *v;
    }

  return sum;
}
