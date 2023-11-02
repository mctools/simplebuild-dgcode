#include "SimpleHistsUtils/AutoBinHistCollection.hh"

namespace SimpleHists {

  AutoBinHist1D* AutoBinHistCollection::book1D(unsigned nbins, const std::string& key)
  {
    Hist1D * h = HistCollection::book1D(nbins,0.0,1.0,key);
    auto ah = new AutoBinHist1D(h);
    ah->autoFit(m_autofit);
    m_autobins.emplace_back(ah);
    return m_autobins.back().get();
  }

  AutoBinHist1D* AutoBinHistCollection::book1D(const std::string& title,unsigned nbins, const std::string& key)
  {
    Hist1D * h = HistCollection::book1D(title,nbins,0.0,1.0,key);
    auto ah = new AutoBinHist1D(h);
    ah->autoFit(m_autofit);
    m_autobins.emplace_back(ah);
    return m_autobins.back().get();
  }

  AutoBinHistCollection::AutoBinHistCollection( AutoBinHistCollection && o )
    : HistCollection(std::move(o)), m_autobins(std::move(o.m_autobins))
  {
  }

  AutoBinHistCollection & AutoBinHistCollection::operator= ( AutoBinHistCollection && rh )
  {
    HistCollection::operator=(std::move(rh));
    m_autobins.clear();
    m_autobins = std::move(rh.m_autobins);
    return *this;
  }

  void AutoBinHistCollection::saveToFile(const std::string& filename, bool allowOverwrite) const
  {
    //    for(auto& a : m_autobins) {
  for (auto it = m_autobins.begin(); it!= m_autobins.end(); ++it) { auto& a = *it;
      a->flush();
  }
    HistCollection::saveToFile(filename,allowOverwrite);
  }

  AutoBinHistCollection::~AutoBinHistCollection()
  {
  }

}
