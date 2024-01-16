#include "SimpleHistsUtils/Sampler.hh"
#include "SimpleHists/Hist1D.hh"
#include <stdexcept>
#include <cassert>
#include <algorithm>
#include <functional>

SimpleHists::Sampler::Sampler(Hist1D*h,double scalefact)
{
  reinit(h,scalefact);
}

SimpleHists::Sampler::~Sampler()
{
}

void SimpleHists::Sampler::reinit(SimpleHists::Hist1D*h,double scalefact)
{
  m_cumul.clear();
  m_edges.clear();

  if (h->empty())
    throw std::runtime_error("SimpleHists::Sampler ERROR: Can't initialise from empty histogram");

  const double fmin(h->getMinFilled());
  const double fmax(h->getMaxFilled());
  const int nbins = static_cast<int>(h->getNBins());
  const double * bincont = h->rawContents();
  const int ibin_fmin = h->valueToBin(fmin);//-1=underflow, nbins=overflow
  const int ibin_fmax = h->valueToBin(fmax);//-1=underflow, nbins=overflow

  m_cumul.reserve(nbins+2);
  m_edges.reserve(nbins+3);

  double e0, e1(0), c, cc(0);
  for (int i=ibin_fmin;i<=ibin_fmax;++i) {
    if (i==-1) {
      //underflow bin (so fmin must be in underflow)
      assert(i==ibin_fmin);
      e0 = fmin;
      e1 = h->getBinLower(0);
      c = h->getUnderflow();
    } else if (i==nbins) {
      //overflow bin  (so fmax must be in overflow)
      assert(i==ibin_fmax);
      e0 = h->getBinUpper(nbins-1);
      e1 = fmax;
      c = h->getOverflow();
    } else {
      //internal bin:
      e0 = h->getBinLower(i);
      e1 = h->getBinUpper(i);
      c = bincont[i];
    }
    //If fmin/fmax is in this bin, move in edges:
    if (i==ibin_fmin)
      e0 = fmin;
    if (i==ibin_fmax)
      e1 = fmax;
    if (!c&&!m_cumul.empty()&&!m_cumul.back())
      continue;//two successive empty bins in a row
    m_cumul.push_back(cc += c);
    m_edges.push_back(e0*scalefact);
  }
  assert(!m_edges.empty());
  m_edges.push_back(e1*scalefact);

  //normalise and accumulate contents, and tighten memory usage in the process:
  //std::vector<double> tmp;
  //tmp.resize(m_cumul.size(),0.0);
  double normfact = 1.0 / m_cumul.back();
  for( auto& e : m_cumul )
    e *= normfact;
  m_cumul.shrink_to_fit();
  if (m_edges.size()!=m_edges.capacity()) {
    std::vector<double> tmp2;
    tmp2.reserve(m_edges.size());
    tmp2.assign(m_edges.begin(),m_edges.end());
    assert(tmp2.size()==m_edges.size());
    m_edges.swap(tmp2);
  }
  assert(m_edges.size()==m_cumul.size()+1);
}

double SimpleHists::Sampler::sample(double rand) const
{
  if (rand<=0.0)
    return m_edges.front();
  auto it = std::lower_bound(m_cumul.begin(),m_cumul.end(),rand);
  if (it==m_cumul.end())
    return m_edges.back();
  double r0 = it==m_cumul.begin() ? 0.0 : *(it-1);
  double rand2 = (rand-r0)/(*it-r0);
  const double * e0 = &m_edges[it-m_cumul.begin()];
  const double e1 = *(e0+1);
  double res = (*e0)*(1.0-rand2) + rand2* (e1);
  //Return result, with extra protection against rounding errors going outside
  //the chosen edges:
  return (res<*e0?*e0:(res>e1?e1:res));
}

