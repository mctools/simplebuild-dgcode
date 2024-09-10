#include "SimpleHistsUtils/AutoBinHist1D.hh"
#include <stdexcept>
#include <algorithm>

SimpleHists::AutoBinHist1D::AutoBinHist1D(Hist1D* h, size_t nwait)
  : m_h(h),
    m_wait(nwait),
    m_autofit_percentile(0)
{
  if (!h||!h->empty())
    throw std::runtime_error("AutoBinHist1D constructor requires non-null pointer to empty Hist1D instance.");
  if (m_wait) {
    m_db.reserve(m_wait);
  } else {
    m_wait = 1;
    flush();
  }
}

SimpleHists::AutoBinHist1D::~AutoBinHist1D()
{
  if (!m_db.empty()) flush(); // cppcheck-suppress throwInNoexceptFunction

}

void SimpleHists::AutoBinHist1D::autoFit( double percentile )
{
  if (percentile<0.01||percentile>1.0)
    throw std::runtime_error("percentile value out of range");
  m_autofit_percentile = percentile;
}

void SimpleHists::AutoBinHist1D::flush()
{
  if (m_wait==0)
    return;
  if (!m_h->empty())
    throw std::runtime_error("AutoBinHist1D only works if the histogram contents are not modified directly.");
  m_wait = 0;

  double vmin = 0.0;
  double vmax = 1.0;
  if (!m_db.empty()&&m_autofit_percentile>0.0&&m_autofit_percentile<1.0) {
    double ignore = (1.0-m_autofit_percentile)*0.5;
    auto dbcpy = m_db;
    std::sort(dbcpy.begin(),dbcpy.end());
    double sumtot(0.0), sum(0);
    //  for (auto& v : dbcpy) {
    for (auto it = dbcpy.begin(); it!= dbcpy.end(); ++it) { auto& v = *it;
      sumtot += (v.second?v.second:1.0);
    }
    //  for (auto& v : dbcpy) {
    for (auto it = dbcpy.begin(); it!= dbcpy.end(); ++it) { auto& v = *it;
      sum += (v.second?v.second:1.0);
      if (sum<sumtot*ignore)
        vmin = v.first;
      if (sum>sumtot*(1-ignore)) {
        vmax = v.first;
        break;
      }
    }
  } else {
    if (!m_db.empty())
      vmin = vmax = m_db.front().first;
    //  for (auto& v : m_db) {
    for (auto it = m_db.begin(); it!= m_db.end(); ++it) { auto& v = *it;
      if (vmin > v.first) vmin = v.first;
      if (vmax < v.first) vmax = v.first;
    }
  }

  double dd = vmax-vmin;
  if (dd>0.0) {
    vmin -= 0.01*dd;
    vmax += 0.01*dd;
  } else {
    vmax = vmin + 1.0;
  }

  m_h->resetAndRebin(m_h->getNBins(), vmin, vmax);
  //  for (auto& v : m_db) {
  for (auto it = m_db.begin(); it!= m_db.end(); ++it) { auto& v = *it;
    if (v.second==0)
      m_h->fill(v.first);
    else
      m_h->fill(v.first,v.second);
  }
  m_db.clear();
  std::vector<std::pair<double,double> > dummy;
  m_db.swap(dummy);
}
