#ifndef SimpleHists_Hist1D_hh
#define SimpleHists_Hist1D_hh

#include "SimpleHists/HistBase.hh"
#include <cmath>

namespace SimpleHists {

  class Hist1D final : public HistBase {
  public:

    Hist1D(unsigned nbins, double xmin, double xmax);
    Hist1D(const std::string& title,unsigned nbins, double xmin, double xmax);
    Hist1D(const std::string& serialised_data);
    virtual ~Hist1D();

    //TODO: ZLabel as well:
    //Annotations (for plots):
    // const std::string& getZLabel() const;
    // void setZLabel(const std::string& l);

    unsigned dimension() const override { return 1; }
    void dump(bool contents = false, const std::string& prefix = "") const override;

    unsigned getNBins() const;
    double getBinContent(unsigned ibin) const;
    double getBinError(unsigned ibin) const;
    double getBinCenter(unsigned ibin) const;
    double getBinLower(unsigned ibin) const;
    double getBinUpper(unsigned ibin) const;
    double getBinWidth() const;
    double getMinFilled() const;//only well-defined if hist is non-empty
    double getMaxFilled() const;//only well-defined if hist is non-empty
    double getUnderflow() const;
    double getOverflow() const;
    double getXMin() const;
    double getXMax() const;
    int valueToBin(double val) const;//-1=underflow, nbins=overflow

    double getMaxContent() const;

    bool empty() const override;
    double getIntegral() const override;

    double getMean() const;//only well-defined if hist is non-empty
    double getRMS() const;//only well-defined if hist is non-empty
    double getRMSSquared() const;//getRMS()^2 (but slightly faster)

    //Normal filling:
    void fill(double val);
    void fill(double val, double weight);

    //Convenient filling from arrays (not exposed to python where arrays can
    //instead be fed to the fill method):
    void fillMany(const double* vals, unsigned n);
    void fillMany(const double* vals, const double* weights, unsigned n);

    //Multiple fillings - just like N calls to fill(val). Note that
    //concerning errors this is not the same as fill(val,N).
    void fillN(unsigned long N, double val);

    char histType() const override { return 0x01; }
    void serialise(std::string&) const override;

    //Raw access to the contents (might get invalidated by future calls to non-const methods):
    const double * rawContents() const { return m_content; }
    const double * rawErrorsSquared() const { return m_errors ? m_errors : m_content; }

    //Merge contents of another compatible histogram onto this one.
    bool mergeCompatible(const HistBase*) const override;//check this before calling next method
    void merge(const HistBase*) override;

    void setErrorsByContent();//After this errors will be sqrt(content)

    bool isSimilar(const HistBase*) const override;

    void scale(double scalefact) override;

    HistBase* clone() const override;

    void reset() override;
    bool canRebin(unsigned new_nbins) const;
    void rebin(unsigned new_nbins);//Only ok to call if canRebin(..) is true.
    void resetAndRebin(unsigned nbins, double xmin, double xmax);

    //Find bin containing a given percentile (percentile=0.5 is the
    //median). Return bin number 0..nbins-1 (if median inside bin ranges), -1
    //(underflow), nbins (overflow).
    //
    //This is only safe to call when histogram is not empty
    int getPercentileBin(double percentile) const;

    //Sum contents from bin1 to bin2 (both bins included). To include underflow
    //content, start from bin1=-1, and to include overflow content, end at
    //bin2=nbins.
    double getBinSum(int bin1, int bin2);

  private:
    void init(unsigned nbins, double xmin, double xmax);

    //Copy/assignment is forbidden:
    Hist1D( const Hist1D & );
    Hist1D & operator= ( const Hist1D & );

    //Use union for persistent data-members for metadata:
    struct PersistifiedData {
      double xmin;
      double xmax;
      double sumW;
      double sumWX;
      double rmsstate;//see comments in hist_stats.hh
      double underflow;
      double overflow;
      float minfilled;//FIXME: This should become double!!
      float maxfilled;//FIXME: This should become double!!
      std::uint32_t nbins;
    };
    union {
      PersistifiedData m_data;
      char m_serialdata[sizeof(double)*7+sizeof(float)*2+sizeof(std::uint32_t)];
    };

    //non-persistent metadata which can be recreated:
    double m_invDelta;
    double m_delta;

    void recalcNonPersistentVars();

    //Bin contents:
    double * m_content;
    double * m_errors;//sum weight-squares for each bin
    void initErrors();
  };

}

#include "SimpleHists/Hist1D.icc"

#endif
