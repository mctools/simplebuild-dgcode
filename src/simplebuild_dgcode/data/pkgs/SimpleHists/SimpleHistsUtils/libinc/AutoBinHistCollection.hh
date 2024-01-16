#ifndef SimpleHists_AutoBinHistCollection_hh
#define SimpleHists_AutoBinHistCollection_hh

#include "SimpleHists/HistCollection.hh"
#include "SimpleHistsUtils/AutoBinHist1D.hh"
#include <vector>
#include <memory>

namespace SimpleHists {

  //Version of HistCollection which adds a version book1D which does not require
  //specification of bin-ranges, and which returns an AutoBinHist1D pointer
  //which can be used for filling as normally, and which will internally taking
  //care of rebinning the actual Hist1D instance to getting bin-ranges adapted
  //to data.

  class AutoBinHistCollection : public HistCollection {
  public:
    AutoBinHistCollection() : HistCollection(), m_autofit(1.0) {}
    AutoBinHistCollection(const std::string&fn) : HistCollection(fn) {}
    virtual ~AutoBinHistCollection();

    AutoBinHist1D* book1D(unsigned nbins, const std::string& key);
    AutoBinHist1D* book1D(const std::string& title,unsigned nbins, const std::string& key);
    using HistCollection::book1D;

    //Change default autofit value for AutoBinHist1D histograms upon booking:
    void setAutoFitDefault( double percentile = 0.99) { m_autofit = percentile; }

    //persistification:
    virtual void saveToFile(const std::string& filename, bool allowOverwrite = false) const;

    AutoBinHistCollection( AutoBinHistCollection && );
    AutoBinHistCollection & operator= ( AutoBinHistCollection && );
  private:
    std::vector<std::unique_ptr<AutoBinHist1D>> m_autobins;
    double m_autofit;
    //Forbid copy/assignment:
    AutoBinHistCollection( const AutoBinHistCollection & ) = delete;
    AutoBinHistCollection & operator= ( const AutoBinHistCollection & ) = delete;
  };

}

#endif
