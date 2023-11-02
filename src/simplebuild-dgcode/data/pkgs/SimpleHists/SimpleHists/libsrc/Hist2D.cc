#include "SimpleHists/Hist2D.hh"
#include "Utils/PackSparseVector.hh"
#include <stdexcept>
#include <cstring>//for memset
#include <cmath>//for sqrt
#include "hist_stats.hh"
#include <vector>

SimpleHists::Hist2D::~Hist2D()
{
  delete[] m_content;
}

void SimpleHists::Hist2D::dump(bool contents,const std::string&prefix) const
{
  const char * p = prefix.c_str();
  printf("%sHist2D(nbinsx=%i,xmin=%g,xmax=%g,nbinsy=%i,ymin=%g,ymax=%g):\n",
         p,getNBinsX(),getXMin(),getXMax(),getNBinsY(),getYMin(),getYMax());
  dumpBase(p);
  printf("%s  integral   : %g\n",p,getIntegral());
  if (!empty()) {
    printf("%s  meanx      : %g\n",p,getMeanX());
    printf("%s  rmsx       : %g\n",p,getRMSX());
    printf("%s  meany      : %g\n",p,getMeanY());
    printf("%s  rmsy       : %g\n",p,getRMSY());
    printf("%s  covxy      : %g\n",p,getCovariance());
    printf("%s  corxy      : %g\n",p,getCorrelation());
    printf("%s  minfilledx : %g\n",p,getMinFilledX());
    printf("%s  maxfilledx : %g\n",p,getMaxFilledX());
    printf("%s  minfilledy : %g\n",p,getMinFilledY());
    printf("%s  maxfilledy : %g\n",p,getMaxFilledY());
  } else {
    printf("%s  meanx      : <n/a>\n",p);
    printf("%s  rmsy       : <n/a>\n",p);
    printf("%s  meanx      : <n/a>\n",p);
    printf("%s  rmsy       : <n/a>\n",p);
    printf("%s  covxy      : <n/a>\n",p);
    printf("%s  corxy      : <n/a>\n",p);
    printf("%s  minfilledx : <n/a>\n",p);
    printf("%s  maxfilledx : <n/a>\n",p);
    printf("%s  minfilledy : <n/a>\n",p);
    printf("%s  maxfilledy : <n/a>\n",p);
  }
  printf("%s  underflowx : %g\n",p,getUnderflowX());
  printf("%s  overflowx  : %g\n",p,getOverflowX());
  printf("%s  underflowy : %g\n",p,getUnderflowY());
  printf("%s  overflowy  : %g\n",p,getOverflowY());
  if (contents) {
    for (unsigned ibinx = 0; ibinx < getNBinsX(); ++ibinx)
      for (unsigned ibiny = 0; ibiny < getNBinsY(); ++ibiny)
        printf("%s  content[(ix,iy)=(%i,%i)] : %g\n",p,ibinx,ibiny,getBinContent(ibinx,ibiny));
  }
}

namespace SimpleHists {
  class RawBufVect {
    //Adaptor class which makes a raw array look like a vector.
  public:
    RawBufVect(size_t n, double * buf) : m_buf(buf), m_size(n) {}
    ~RawBufVect(){}
    size_t size() const { return m_size; }
    void resize(size_t) { throw std::runtime_error("RawBufVect can not be resized"); }
    double operator[] ( size_t i ) const { return m_buf[i]; }
    double& operator[] ( size_t i ) { return m_buf[i]; }
    typedef double value_type;
  private:
    double * m_buf;
    size_t m_size;
  };

  struct FlexBuf {
    void write(unsigned char* buf, unsigned buflen)
    {
      for (unsigned i = 0; i < buflen; ++i)
        buffer.push_back(buf[i]);
    }
    std::vector<char> buffer;
  };

  class RawByteBuf {
    //Adaptor class which constructs a dataProvider from a raw array.
  public:
    RawByteBuf(size_t n, const char * buf) : m_buf(buf), m_left(n) {}
    ~RawByteBuf(){}
    unsigned read(unsigned char* buf, unsigned buflen) {
      if (buflen>m_left)
        return 0;
      std::memcpy(buf,m_buf,buflen);
      m_buf += buflen;
      m_left -= buflen;
      return buflen;
    }
    size_t left() const { return m_left; }
  private:
    const char * m_buf;
    size_t m_left;
  };

}

SimpleHists::Hist2D::Hist2D(const std::string& serialised_data)
  : HistBase()
{
  unsigned offset;
  char version;
  deserialiseBase(serialised_data,offset,version);
  assert(version==0x01||version==0x02);//should have already been checked in deserialiseBase

  std::memcpy(&(m_serialdata[0]),&(serialised_data[offset]),sizeof(m_serialdata));
  offset += sizeof(m_serialdata);

  unsigned ncells = m_data.nbinsx*m_data.nbinsy;

  m_content = new double[ncells];
  if (version==0x01) {
    if (offset+ncells*sizeof(double)!=serialised_data.size())
      throw std::runtime_error("Hist2D: Histogram deserialisation failed! (data size error)");
    std::memcpy(reinterpret_cast<char*>(m_content),&(serialised_data[offset]),ncells*sizeof(double));
  } else {
    std::memset(m_content,0,ncells*sizeof(double));
    RawBufVect rbv(ncells,m_content);
    RawByteBuf rbb(serialised_data.size()-offset,&(serialised_data[offset]));
    Utils::PackSparseVector::read(rbv,
                                  std::bind(&RawByteBuf::read,&rbb,std::placeholders::_1,std::placeholders::_2));
  }

  //setup non-persistent vars:
  recalcNonPersistentVars();
}

void SimpleHists::Hist2D::serialise(std::string& buf) const
{
  //Write packed version of content into temporary flex-buffer:
  size_t ncells = m_data.nbinsx*m_data.nbinsy;
  RawBufVect rbv(ncells,m_content);
  FlexBuf fbuf;
  fbuf.buffer.reserve(4096);
  Utils::PackSparseVector::write(rbv,std::bind(&FlexBuf::write,&fbuf,std::placeholders::_1,std::placeholders::_2));
  size_t nbase = serialisedBaseSize();
  size_t n = fbuf.buffer.size() + sizeof(m_serialdata);
  buf.resize(n+nbase);//a bit wasteful initialisation
  serialiseBaseToBuffer(&(buf[0]));
  std::memcpy(&(buf[nbase]),&(m_serialdata[0]),sizeof(m_serialdata));
  std::memcpy(&(buf[nbase+sizeof(m_serialdata)]),&(fbuf.buffer[0]),fbuf.buffer.size());
}

void SimpleHists::Hist2D::recalcNonPersistentVars()
{
  m_deltaX = (m_data.xmax-m_data.xmin)/m_data.nbinsx;
  m_deltaY = (m_data.ymax-m_data.ymin)/m_data.nbinsy;
  m_invDeltaX = 1.0/m_deltaX;
  m_invDeltaY = 1.0/m_deltaY;
  unsigned ncells = m_data.nbinsx*m_data.nbinsy;
  if (ncells/m_data.nbinsx!=m_data.nbinsy)
    throw std::runtime_error("Hist2D: Overflow in nbinsx*nbinsy (too many bins?)");
  if (ncells>250000000)
    throw std::runtime_error("Hist2D: Too many bins");
}

double SimpleHists::Hist2D::getMinFilledX() const
{
  if (!getIntegral())
    throw std::runtime_error("Hist2D: min filled not well defined in empty histograms");
  return m_data.minfilledx;
}

double SimpleHists::Hist2D::getMaxFilledX() const
{
  if (!getIntegral())
    throw std::runtime_error("Hist2D: max filled not well defined in empty histograms");
  return m_data.maxfilledx;
}

double SimpleHists::Hist2D::getMinFilledY() const
{
  if (!getIntegral())
    throw std::runtime_error("Hist2D: min filled not well defined in empty histograms");
  return m_data.minfilledy;
}

double SimpleHists::Hist2D::getMaxFilledY() const
{
  if (!getIntegral())
    throw std::runtime_error("Hist2D: max filled not well defined in empty histograms");
  return m_data.maxfilledy;
}

double SimpleHists::Hist2D::getMeanX() const
{
  return calc_mean(m_data.sumW,m_data.sumWX);
}

double SimpleHists::Hist2D::getRMSX() const
{
#if SIMPLEHISTS_ROOT_STYLE_RMS
  return calc_rms(m_data.sumW,m_data.sumWX,m_data.rmsstateX);
#else
  return calc_rms(m_data.sumW,m_data.rmsstateX);
#endif
}

double SimpleHists::Hist2D::getRMSSquaredX() const
{
#if SIMPLEHISTS_ROOT_STYLE_RMS
  return calc_rms2(m_data.sumW,m_data.sumWX,m_data.rmsstateX);
#else
  return calc_rms2(m_data.sumW,m_data.rmsstateX);
#endif
}

double SimpleHists::Hist2D::getMeanY() const
{
  return calc_mean(m_data.sumW,m_data.sumWY);
}

double SimpleHists::Hist2D::getRMSY() const
{
#if SIMPLEHISTS_ROOT_STYLE_RMS
  return calc_rms(m_data.sumW,m_data.sumWY,m_data.rmsstateY);
#else
  return calc_rms(m_data.sumW,m_data.rmsstateY);
#endif
}

double SimpleHists::Hist2D::getRMSSquaredY() const
{
#if SIMPLEHISTS_ROOT_STYLE_RMS
  return calc_rms2(m_data.sumW,m_data.sumWY,m_data.rmsstateY);
#else
  return calc_rms2(m_data.sumW,m_data.rmsstateY);
#endif
}

double SimpleHists::Hist2D::getCovariance() const
{
  return calc_covxy(m_data.sumW,m_data.covstate);
}

double SimpleHists::Hist2D::getCorrelation() const
{
  if (!m_data.sumW)
    throw std::runtime_error("correlation not well defined in empty histograms");
  const double r= getRMSX()*getRMSY();
  return r ? calc_covxy(m_data.sumW,m_data.covstate)/r : 0.0;
}

void SimpleHists::Hist2D::init(unsigned nbinsx, double xmin, double xmax,
                               unsigned nbinsy, double ymin, double ymax)
{
  //Sanity check input data:
  if (!nbinsx||!nbinsy) throw std::runtime_error("Hist2D: Can not book histogram with zero bins");
  if (xmin!=xmin) throw std::runtime_error("Hist2D: Lower bin range in X is nan (not-a-number)");
  if (xmax!=xmax) throw std::runtime_error("Hist2D: Upper bin range in X is nan (not-a-number)");
  if (xmax<=xmin) throw std::runtime_error("Hist2D: Upper bin range of X must be greater than lower bin range of X");
  if (ymin!=ymin) throw std::runtime_error("Hist2D: Lower bin range in Y is nan (not-a-number)");
  if (ymax!=ymax) throw std::runtime_error("Hist2D: Upper bin range in Y is nan (not-a-number)");
  if (ymax<=ymin) throw std::runtime_error("Hist2D: Upper bin range of Y must be greater than lower bin range of Y");

  unsigned ncells = nbinsx*nbinsy;
  if (ncells/nbinsx!=nbinsy||ncells/nbinsy!=nbinsx||ncells>250000000)// "ncells/nbinsx!=nbinsy" is a sign of overflow
    throw std::runtime_error("Hist2D Too many bins in histogram - max allowed is nbinsx*nbinsy = 250e6");

  //metadata:
  std::memset(m_serialdata,0,sizeof(m_serialdata));
  m_data.nbinsx = nbinsx;
  m_data.xmin = xmin;
  m_data.xmax = xmax;
  m_data.nbinsy = nbinsy;
  m_data.ymin = ymin;
  m_data.ymax = ymax;
  m_data.maxfilledx = m_data.maxfilledy = -1;//use max<min to indicate no fills yet
  m_data.minfilledx = m_data.minfilledy = 1;

  //content
  m_content = new double[ncells];
  std::memset(m_content,0,sizeof(double)*ncells);

  //non-persistent metadata
  recalcNonPersistentVars();
}

bool SimpleHists::Hist2D::mergeCompatible(const HistBase*o) const
{
  if (!HistBase::mergeCompatible(o))
    return false;
  const Hist2D* other_2d = dynamic_cast<const Hist2D*>(o);
  if (!other_2d)
    return false;
  //todo: zlabel!
  if (getNBinsX()!=other_2d->getNBinsX()) return false;
  if (getNBinsY()!=other_2d->getNBinsY()) return false;
  if (getXMin()!=other_2d->getXMin()) return false;
  if (getXMax()!=other_2d->getXMax()) return false;
  if (getYMin()!=other_2d->getYMin()) return false;
  if (getYMax()!=other_2d->getYMax()) return false;
  return true;//succes!
}

void SimpleHists::Hist2D::merge(const HistBase*obase)
{
  assert(obase);
  if (!mergeCompatible(obase))
    throw std::runtime_error("Attempting to merge incompatible 2D histograms");

  const Hist2D * o = dynamic_cast<const Hist2D*>(obase);
  assert(o);

  //statistics:
  merge_covxy(m_data.covstate, m_data.sumW, m_data.sumWX, m_data.sumWY,
              o->m_data.covstate, o->m_data.sumW, o->m_data.sumWX, o->m_data.sumWY);

  double fakesumw(m_data.sumW);
  merge_stats(m_data.sumW, m_data.sumWX, m_data.rmsstateX,
              o->m_data.sumW, o->m_data.sumWX, o->m_data.rmsstateX);
  merge_stats(fakesumw, m_data.sumWY, m_data.rmsstateY,
              o->m_data.sumW, o->m_data.sumWY, o->m_data.rmsstateY);

  merge_maxmin(m_data.minfilledx, m_data.maxfilledx,
               o->m_data.minfilledx, o->m_data.maxfilledx);
  merge_maxmin(m_data.minfilledy, m_data.maxfilledy,
               o->m_data.minfilledy, o->m_data.maxfilledy);

  m_data.underflowx += o->m_data.underflowx;
  m_data.overflowx  += o->m_data.overflowx;
  m_data.underflowy += o->m_data.underflowy;
  m_data.overflowy  += o->m_data.overflowy;

  //contents:
  double * it = m_content;
  double * itE = m_content + m_data.nbinsx*m_data.nbinsy;
  double * itO = o->m_content;
  while (it!=itE)
    *(it++) += *(itO++);

  assert((itO - (o->m_content)) == (int)(o->m_data.nbinsx*o->m_data.nbinsy));
}

void SimpleHists::Hist2D::fill(double valx,double valy)
{
  int ibinx = valueToBinX(valx);
  int ibiny = valueToBinY(valy);
  update_maxmin_filled(m_data.minfilledx,m_data.maxfilledx,valx);
  update_maxmin_filled(m_data.minfilledy,m_data.maxfilledy,valy);

  update_covxy_on_fill(m_data.sumW, m_data.sumWX, m_data.sumWY,
                       m_data.covstate, valx,valy);
  double fakesumw(m_data.sumW);
  update_stats_on_fill(m_data.sumW,m_data.sumWX,m_data.rmsstateX,valx);
  update_stats_on_fill(fakesumw,m_data.sumWY,m_data.rmsstateY,valy);

  bool inside(true);

  if (ibinx<0) { ++(m_data.underflowx); inside=false; }
  else if (ibinx>=static_cast<int>(m_data.nbinsx)) { ++(m_data.overflowx); inside=false; }

  if (ibiny<0) { ++(m_data.underflowy); inside=false; }
  else if (ibiny>=static_cast<int>(m_data.nbinsy)) { ++(m_data.overflowy); inside=false; }

  if (inside) {
    unsigned ic = icell(static_cast<int>(ibinx),static_cast<int>(ibiny));
    ++(m_content[ic]);
  }
}

void SimpleHists::Hist2D::fill(double valx, double valy, double weight)
{
  assert(!(weight!=weight)&&"Hist2D ERROR: NAN in input weight!");
  if (weight<=0) {
    if (weight==0)
      return;
    throw std::runtime_error("Hist2D: Fill with negative weights gives ill-defined statistics");
  }
  int ibinx = valueToBinX(valx);
  int ibiny = valueToBinY(valy);
  update_maxmin_filled(m_data.minfilledx,m_data.maxfilledx,valx);
  update_maxmin_filled(m_data.minfilledy,m_data.maxfilledy,valy);
  update_covxy_on_fill(m_data.sumW, m_data.sumWX, m_data.sumWY,
                       m_data.covstate, valx,valy,weight);
  double fakesumw(m_data.sumW);
  update_stats_on_fill(m_data.sumW,m_data.sumWX,m_data.rmsstateX,valx,weight);
  update_stats_on_fill(fakesumw,m_data.sumWY,m_data.rmsstateY,valy,weight);

  bool inside(true);

  if (ibinx<0) { m_data.underflowx+=weight; inside=false; }
  else if (ibinx>=static_cast<int>(m_data.nbinsx)) { m_data.overflowx+=weight; inside=false; }

  if (ibiny<0) { m_data.underflowy+=weight; inside=false; }
  else if (ibiny>=static_cast<int>(m_data.nbinsy)) { m_data.overflowy+=weight; inside=false; }

  if (inside) {
    unsigned ic = icell(static_cast<int>(ibinx),static_cast<int>(ibiny));
    m_content[ic] += weight;
  }
}

void SimpleHists::Hist2D::fillMany(const double* valsx, const double* valsy, unsigned n)
{
  const double *vx(valsx);
  const double *vxE(valsx+n);
  const double *vy(valsy);
  for(;vx!=vxE;++vx,++vy)
    fill(*vx,*vy);
}

void SimpleHists::Hist2D::fillMany(const double* valsx, const double* valsy, const double* weights, unsigned n)
{
  const double *vx(valsx);
  const double *vxE(valsx+n);
  const double *w(weights);
  const double *vy(valsy);
  for(;vx!=vxE;++vx,++vy,++w)
    fill(*vx,*vy,*w);
}

void SimpleHists::Hist2D::scale(double a)
{
  if (a==1.0)
    return;
  if (a<=0)
    throw std::runtime_error("scale factor must be >0");
  m_data.sumW *= a;
  m_data.sumWX *= a;
  m_data.rmsstateX *= a;
  m_data.sumWY *= a;
  m_data.rmsstateY *= a;
  m_data.covstate *= a;
  m_data.underflowx *= a;
  m_data.overflowx *= a;
  m_data.underflowy *= a;
  m_data.overflowy *= a;

  //max/minfilled are not affected when scaled

  double *v(m_content), *vE(m_content+m_data.nbinsx*m_data.nbinsy);
  for(;v!=vE;++v)
    *v *= a;
}

#include "floatcompat.hh"

bool SimpleHists::Hist2D::isSimilar(const HistBase* obase) const
{
  if (!HistBase::isSimilar(obase))
    return false;
  //non-float metadata first:
  const Hist2D * o = dynamic_cast<const Hist2D*>(obase);
  assert(o);//HistBase already checked histType
  if (m_data.nbinsx!=o->m_data.nbinsx) return false;
  if (m_data.nbinsy!=o->m_data.nbinsy) return false;

  //float metadata:
  if (!floatCompatible(m_data.xmin,o->m_data.xmin)) return false;
  if (!floatCompatible(m_data.xmax,o->m_data.xmax)) return false;
  if (!floatCompatible(m_data.ymin,o->m_data.ymin)) return false;
  if (!floatCompatible(m_data.ymax,o->m_data.ymax)) return false;
  if (!floatCompatible(m_data.sumW,o->m_data.sumW)) return false;
  if (!floatCompatible(m_data.sumWX,o->m_data.sumWX)) return false;
  if (!floatCompatible(m_data.rmsstateX,o->m_data.rmsstateX)) return false;
  if (!floatCompatible(m_data.sumWY,o->m_data.sumWY)) return false;
  if (!floatCompatible(m_data.rmsstateY,o->m_data.rmsstateY)) return false;
  if (!floatCompatible(m_data.covstate,o->m_data.covstate)) return false;
  if (!floatCompatible(m_data.underflowx,o->m_data.underflowx)) return false;
  if (!floatCompatible(m_data.overflowx,o->m_data.overflowx)) return false;
  if (!floatCompatible(m_data.underflowy,o->m_data.underflowy)) return false;
  if (!floatCompatible(m_data.overflowy,o->m_data.overflowy)) return false;

  //contents:
  {
    const double *it(m_content);
    const double *itE(m_content+m_data.nbinsx*m_data.nbinsy);
    const double *ito(o->m_content);
    for (;it!=itE;++it,++ito) {
      if (!floatCompatible(*it,*ito))
        return false;
    }
  }
  return true;
}

SimpleHists::HistBase* SimpleHists::Hist2D::clone() const
{
  Hist2D * h = new Hist2D(getTitle(),
                          m_data.nbinsx,m_data.xmin,m_data.xmax,
                          m_data.nbinsy,m_data.ymin,m_data.ymax);
  h->setXLabel(getXLabel());
  h->setYLabel(getYLabel());
  h->setComment(getComment());

  std::memcpy(h->m_serialdata,m_serialdata,sizeof(m_serialdata));

  //non-persistent vars:
  h->m_invDeltaX = m_invDeltaX;
  h->m_deltaX = m_deltaX;
  h->m_invDeltaY = m_invDeltaY;
  h->m_deltaY = m_deltaY;

  std::memcpy(h->m_content,m_content,m_data.nbinsx*m_data.nbinsy*sizeof(double));

  return h;
}

void SimpleHists::Hist2D::reset()
{
  delete[] m_content;
  init(m_data.nbinsx,m_data.xmin,m_data.xmax,
       m_data.nbinsy,m_data.ymin,m_data.ymax);
}
