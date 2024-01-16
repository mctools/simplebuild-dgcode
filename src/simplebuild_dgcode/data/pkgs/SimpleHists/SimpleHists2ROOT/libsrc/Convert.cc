#include "SimpleHists2ROOT/Convert.hh"
#include "SimpleHists/HistCollection.hh"
#include "TH1D.h"
#include "TH2D.h"
#include "TFile.h"
#include <cstring>//memcpy
#include <numeric>//accumulate

namespace SimpleHists {

  void convertBase(const HistBase*hs,TH1*hr,const char* convtype)
  {
    hr->Sumw2();//Although tempting to not always do this, it might result in user confusion if we don't.
    if (!hs->getXLabel().empty())
      hr->GetXaxis()->SetTitle(hs->getXLabel().c_str());
    if (!hs->getYLabel().empty())
      hr->GetYaxis()->SetTitle(hs->getYLabel().c_str());
    if (!hs->getComment().empty())
      printf("SH2ROOT: Warning - Comment lost upon conversion for %s: %s.\n",convtype,hr->GetName());
  }
}

TH1D * SimpleHists::convertToROOT(const SimpleHists::Hist1D* hs, const std::string& root_name)
{
  assert(hs);
  const unsigned nbins(hs->getNBins());
  TH1D * hr = new TH1D(root_name.c_str(),hs->getTitle().c_str(),
                       nbins,hs->getXMin(),hs->getXMax());
  convertBase(hs,hr,"Hist1D->TH1D");
  if (hs->empty())
    return hr;

  //Transfer contents, errors and under/overflow (under/overflow won't get errors assigned):
  const double * hs_errors = const_cast<SimpleHists::Hist1D*>(hs)->rawErrorsSquared();
  const double * hs_contents = const_cast<SimpleHists::Hist1D*>(hs)->rawContents();
  std::memcpy(hr->fArray+1,hs_contents,nbins*sizeof(double));
  std::memcpy(hr->GetSumw2()->fArray+1,hs_errors,nbins*sizeof(double));
  hr->fArray[0] = hs->getUnderflow();
  hr->fArray[nbins+1] = hs->getOverflow();

  //Transfer statistics:
  double sumw = hs->getIntegral();
  assert(sumw);//since hs is not empty
  double mean = hs->getMean();
  double sumwx = mean * sumw;
  double sumwx2  = sumw * (mean*mean + hs->getRMSSquared());
  double sumw2 = 0;
  double nentries = sumw;//Not precisely like in ROOT where over/underflows are not always counted and where nentries are un-weighed
  if (hs_contents!=hs_errors) {
    printf("SH2ROOT: Warning - Non-unit weights fills detected meaning inaccurate sum(w^2) stats for Hist1D: %s.\n",root_name.c_str());
    //some or all fills were without unit weight
    sumw2=0.0;
  } else {
    //all fills were with unit weight (or all in over/under flow, but we assume the case is not that extreme)
    sumw2 = sumw;
  }
  double stats[4];
  stats[0] = sumw;
  stats[1] = sumw2;
  stats[2] = sumwx;
  stats[3] = sumwx2;
  hr->PutStats(stats);
  hr->SetEntries(nentries);
  return hr;
}

TH2D * SimpleHists::convertToROOT(const SimpleHists::Hist2D* hs, const std::string& root_name)
{
  assert(hs);
  const unsigned nbinsx(hs->getNBinsX());
  const unsigned nbinsy(hs->getNBinsY());
  TH2D * hr = new TH2D(root_name.c_str(),hs->getTitle().c_str(),
                       nbinsx,hs->getXMin(),hs->getXMax(),
                       nbinsy,hs->getYMin(),hs->getYMax());
  convertBase(hs,hr,"Hist2D->TH2D");
  if (hs->empty())
    return hr;

  //We have to transfer bin contents the hard way, since ROOT has different x-y
  //slicing than SimpleHists (which is designed to be consistent with
  //numpy.histogram2d):
  const int nbinsx_plus2 = nbinsx+2;
  for (unsigned ibinx=0;ibinx<nbinsx;++ibinx){
    for (unsigned ibiny=0;ibiny<nbinsy;++ibiny) {
      int rootbin = (ibinx+1) + nbinsx_plus2 * (ibiny+1);
      double c = hs->getBinContent(ibinx,ibiny);
      hr->SetBinContent(rootbin,c);
      hr->SetBinError(rootbin,sqrt(c));//No real error info is available on
                                       //SimpleHist::Hist2D (since it is not
                                       //normally very useful to plot).
    }
  }

  //We don't really know where to put the underflows, so spread them out along
  //the edges (none in the corners, to avoid confusion):
  double ufx = hs->getUnderflowX() / nbinsy;
  if (ufx) {
    for (int biny=1;biny<=(int)nbinsy;++biny)
      hr->SetBinContent(nbinsx_plus2 * biny,ufx);
  }
  double ofx = hs->getOverflowX() / nbinsy;
  if (ofx) {
    for (int biny=1;biny<=(int)nbinsy;++biny)
      hr->SetBinContent(nbinsx + 1 + nbinsx_plus2 * biny,ofx);
  }
  double ufy = hs->getUnderflowY() / nbinsx;
  if (ufy) {
    for (int binx=1;binx<=(int)nbinsx;++binx)
      hr->SetBinContent(binx,ufy);
  }
  double ofy = hs->getOverflowY() / nbinsx;
  if (ofy) {
    for (int binx=1;binx<=(int)nbinsx;++binx)
      hr->SetBinContent(binx + nbinsx_plus2 * (nbinsy+1),ufy);
  }
  //Transfer statistics:
  double sumw = hs->getIntegral();
  assert(sumw);//since hs is not empty
  double meanx = hs->getMeanX();
  double sumwx = meanx * sumw;
  double sumwx2  = sumw * (meanx*meanx + hs->getRMSSquaredX());
  double meany = hs->getMeanY();
  double sumwy = meany * sumw;
  double sumwy2  = sumw * (meany*meany + hs->getRMSSquaredY());
  double sumwxy = sumw * ( hs->getCovariance() + sumwx*sumwy / (sumw*sumw) );

  //These we can't really know from SimpleHist::Hist2D's:
  double sumw2 = sumw;//wrong if weighted fills

  double stats[7];
  stats[0] = sumw;
  stats[1] = sumw2;
  stats[2] = sumwx;
  stats[3] = sumwx2;
  stats[4] = sumwy;
  stats[5] = sumwy2;
  stats[6] = sumwxy;
  hr->PutStats(stats);
  hr->SetEntries(sumw);//Not precisely like in ROOT where over/underflows are not always counted and where nentries are un-weighed

  return hr;
}

TH1D * SimpleHists::convertToROOT(const SimpleHists::HistCounts* hs, const std::string& root_name)
{
  assert(hs);
  std::list<HistCounts::Counter> counters;
  const_cast<HistCounts*>(hs)->getCounters(counters);
  unsigned n(counters.empty()? 1 : counters.size());
  TH1D * hr = new TH1D(root_name.c_str(),hs->getTitle().c_str(),n,0,n);
  convertBase(hs,hr,"HistCounts->TH2D");
  if (counters.empty())
    return hr;

  auto itE = counters.end();
  int ibin(1);
  unsigned comments(0);
  for (auto it = counters.begin();it!=itE;++it,++ibin) {
    hr->GetXaxis()->SetBinLabel(ibin,it->getLabel().c_str());
    hr->SetBinContent(ibin,it->getValue());
    hr->SetBinError(ibin,it->getError());
    if (!it->getComment().empty())
      ++comments;
  }

  if (comments)
    printf("SH2ROOT: Warning - %i counter comments lost upon conversion for HistCounts->TH1D: %s.\n",comments,hr->GetName());

  //stats are meaningless for this kind of histograms:
  double stats[4];
  stats[0] = 1.0;
  stats[1] = stats[2] = stats[3] = 0.0;
  hr->PutStats(stats);
  hr->SetEntries(1.0);
  return hr;
}

//Version operating on base classes, dispatching to the appropriate of the methods above:
TH1 * SimpleHists::convertToROOT(const SimpleHists::HistBase* h, const std::string& root_name)
{
  assert(h);
  char ht(h->histType());
  if (ht==0x01)
    return convertToROOT(static_cast<const Hist1D*>(h),root_name);
  if (ht==0x02)
    return convertToROOT(static_cast<const Hist2D*>(h),root_name);
  assert(ht==0x03);
  return convertToROOT(static_cast<const HistCounts*>(h),root_name);
}

//Create ROOT files based on SimpleHist collections:
void SimpleHists::convertToROOTFile( const SimpleHists::HistCollection* hc,
                                     const std::string& filename_root )
{
  std::set<std::string> keys;
  hc->getKeys(keys);
  auto itE=keys.end();
  std::vector<TH1*> rh;
  for (auto it=keys.begin();it!=itE;++it)
    rh.push_back(convertToROOT(hc->hist(*it),*it));
  TFile f(filename_root.c_str(),"RECREATE");
  for (auto it=rh.begin();it!=rh.end();++it)
    (*it)->Write();
  f.Close();
  printf("SH2ROOT: Wrote %i histograms to file %s\n",(int)rh.size(),filename_root.c_str());
}

void SimpleHists::convertToROOTFile( const std::string& filename_shist,
                                     const std::string& filename_root )
{
  SimpleHists::HistCollection hc(filename_shist);
  convertToROOTFile(&hc,filename_root);
}
