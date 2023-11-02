#ifndef SimpleHists_Hist2D_hh
#define SimpleHists_Hist2D_hh

#include "SimpleHists/HistBase.hh"

namespace SimpleHists {

  class Hist2D : public HistBase {
  public:

    Hist2D(unsigned nbinsx, double xmin, double xmax,
           unsigned nbinsy, double ymin, double ymax);
    Hist2D(const std::string& title,
           unsigned nbinsx, double xmin, double xmax,
           unsigned nbinsy, double ymin, double ymax);
    Hist2D(const std::string& serialised_data);
    virtual ~Hist2D();

    virtual unsigned dimension() { return 2; }
    virtual void dump(bool contents = false,const std::string& prefix = "") const;

    unsigned getNBinsX() const;
    unsigned getNBinsY() const;
    double getBinContent(unsigned ibinx,unsigned ibiny) const;
    double getBinCenterX(unsigned ibinx) const;
    double getBinLowerX(unsigned ibinx) const;
    double getBinUpperX(unsigned ibinx) const;
    double getBinCenterY(unsigned ibiny) const;
    double getBinLowerY(unsigned ibiny) const;
    double getBinUpperY(unsigned ibiny) const;
    double getBinWidthX() const;
    double getBinWidthY() const;
    double getMinFilledX() const;//only well-defined if hist is non-empty
    double getMaxFilledX() const;//only well-defined if hist is non-empty
    double getMinFilledY() const;//only well-defined if hist is non-empty
    double getMaxFilledY() const;//only well-defined if hist is non-empty
    double getUnderflowX() const;
    double getOverflowX() const;
    double getUnderflowY() const;
    double getOverflowY() const;
    double getXMin() const;
    double getXMax() const;
    double getYMin() const;
    double getYMax() const;
    int valueToBinX(double valx) const;
    int valueToBinY(double valy) const;

    virtual bool empty() const;
    virtual double getIntegral() const;

    //Next six functions are only well-defined when historam is not empty:
    double getMeanX() const;
    double getMeanY() const;
    double getRMSX() const;
    double getRMSY() const;
    double getCovariance() const;
    double getCorrelation() const;//covariance/(rmsx*rmsy) (but 0 if either rmsx or rmsy is 0)
    double getRMSSquaredX() const;
    double getRMSSquaredY() const;

    //Normal filling:
    void fill(double valx,double valy);
    void fill(double valx, double valy, double weight);

    //Convenient filling from arrays (not exposed to python where arrays can
    //instead be fed to the fill method):
    void fillMany(const double* valsx, const double* valsy, unsigned n);
    void fillMany(const double* valsx, const double* valsy, const double* weights, unsigned n);

    virtual char histType() const { return 0x02; }
    virtual void serialise(std::string&) const;

    //Raw access to the contents:
    const double * rawContents() { return m_content; }

    //Merge contents of another compatible histogram onto this one.
    virtual bool mergeCompatible(const HistBase*) const;//check this before calling next method
    virtual void merge(const HistBase*);

    virtual bool isSimilar(const HistBase*) const;

    virtual void scale(double scalefact);

    virtual HistBase* clone() const;

    virtual void reset();

  private:
    void init(unsigned nbinsx, double xmin, double xmax,
              unsigned nbinsy, double ymin, double ymax);

    //Copy/assignment is forbidden:
    Hist2D( const Hist2D & );
    Hist2D & operator= ( const Hist2D & );

    //Use union for persistent data-members for metadata:
    struct PersistifiedData {
      double xmin;
      double xmax;
      double ymin;
      double ymax;
      double sumW;
      double sumWX;
      double rmsstateX;//see comments in hist_stats.hh
      double sumWY;
      double rmsstateY;//see comments in hist_stats.hh
      double covstate;//see comments in hist_stats.hh
      double underflowx;
      double overflowx;
      double underflowy;
      double overflowy;
      float minfilledx;
      float maxfilledx;
      float minfilledy;
      float maxfilledy;
      std::uint32_t nbinsx;
      std::uint32_t nbinsy;
    };
    union {
      PersistifiedData m_data;
      char m_serialdata[sizeof(double)*14+4*sizeof(float)+sizeof(std::uint32_t)*2];
    };

    //non-persistent metadata which can be recreated:
    double m_invDeltaX;
    double m_deltaX;
    double m_invDeltaY;
    double m_deltaY;

    void recalcNonPersistentVars();

    //Bin contents:
    double * m_content;//nbinsx*nbinsy cells
    unsigned icell(unsigned ibinx,unsigned ibiny) const;
  };

}

#include "SimpleHists/Hist2D.icc"

#endif
