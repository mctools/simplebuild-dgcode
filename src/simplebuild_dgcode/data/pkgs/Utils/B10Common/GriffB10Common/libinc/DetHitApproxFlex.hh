#ifndef GriffB10Common_DetHitApproxFlex_hh
#define GriffB10Common_DetHitApproxFlex_hh

#include "GriffB10Common/DetHitApproximation.hh"

// Version which adds a given number of points per step, rather than just 2.
// It can also be used to pretend file is in REDUCED mode, even it is FULL.

class DetHitApproxFlex : public DetHitApproximation {
public:
   DetHitApproxFlex(GriffDataReader* dr,
                    double cluster_outlier_threshold = 1*Units::cm,
                    double hit_edep_threshold = 150*Units::keV,
                    const char * volname = "CountingGas");

  void setPretendReduced(bool p = true);//Results will be as in REDUCED mode, even if Griff file has FULL step info.
  void setNPointsPerPath(unsigned);//default is 2 (as for DetHitApproximation)

  bool pretendReduced() const { return m_segmentsOnly; }
  unsigned nPointsPerPath() const { return m_n; }

  virtual ~DetHitApproxFlex(){}
protected:
  virtual void addPoints(std::vector<Point>&,const GriffDataRead::Segment*);
private:
  virtual void addp( std::vector<Point>&, double edep,
                     const double* p0, double t0,
                     const double* p1, double t1) const;
  bool m_segmentsOnly;
  unsigned m_n;
};

#endif
