#ifndef SimpleHists_AutoBinHist1D_hh
#define SimpleHists_AutoBinHist1D_hh

#include "SimpleHists/Hist1D.hh"
#include <vector>

//Class wrapping a Hist1D instance, catching fills and trying to automatically
//rebin the Hist1D to suitable bin-ranges by temporarily storing a large number
//of filled values in an internal array.

namespace SimpleHists {

  class AutoBinHist1D {
  public:
    AutoBinHist1D(Hist1D* h, size_t nwait = 10000);
    ~AutoBinHist1D();

    void autoFit( double percentile = 0.99);//attempts to ignore outliers affecting bin ranges

    void fill(double val);
    void fill(double val, double weight);
    //todo: fillMany as well.

    HistBase * hist() { return m_h; }
    const HistBase * hist() const { return m_h; }

    void flush();
  private:
    Hist1D * m_h;
    size_t m_wait;
    double m_autofit_percentile;
    std::vector<std::pair<double,double> > m_db;
  };

  inline void AutoBinHist1D::fill(double val)
  {
    if (m_wait) {
      m_db.emplace_back(val,0.0);//0.0 on purpose
      if (!--m_wait) {
        m_wait=1;
        flush();
      }
    } else {
      m_h->fill(val);
    }
  }

  inline void AutoBinHist1D::fill(double val, double weight)
  {
    if (m_wait) {
      if (weight==0)
        return;
      m_db.emplace_back(val,weight);
      if (!--m_wait) {
        m_wait=1;
        flush();
      }
    } else {
      m_h->fill(val,weight);
    }
  }

}

#endif
