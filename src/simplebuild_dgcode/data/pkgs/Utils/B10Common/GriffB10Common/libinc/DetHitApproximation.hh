#ifndef GriffB10Common_DetHitApproximation_hh
#define GriffB10Common_DetHitApproximation_hh

#include "GriffAnaUtils/SegmentIterator.hh"
#include "GriffDataRead/GriffDataReader.hh"
#include "GriffDataRead/Segment.hh"
#include "Units/Units.hh"

// Helper class which looks at energy deposits left in the counting gas and
// determines if a hit was found along with its position and hit-time.
//
// It tries to be relatively simple, but does attempt to remove outliers.
//
// It is not currently designed to work with more than 1 real hit/event, but
// could in principle be adapted.
//
// Strategy: Replace each step (or segment in REDUCED griff mode) with a point at
//           each end (sharing the total energy deposit along the step evenly).
//
//           Recursively find the weighed center of the points while removing
//           outliers. This gives both a hit position and energy.
//
//           To find the hit time, sort the remaining points by time and go
//           through the points until enough energy is found to exceed the
//           threshold. The time of the last point is the hit time.
//
// A future more advanced version of this class could try to do away with the
// point approximation, and probably instead should generate actual ionisation
// clusters. However, such "digitization" simulation would ultimately require
// detailed knowledge of the field setup and counting gas properties.

class DetHitApproximation : public GriffDataRead::BeginEventCallBack{
public:
  ///////////////////////////////////////////////
  //Public interface:
  DetHitApproximation(GriffDataReader*,
                      double cluster_outlier_threshold = 1*Units::cm,
                      double hit_edep_threshold = 150*Units::keV,
                      const char * volname = "CountingGas");
  virtual ~DetHitApproximation();


  //Extract hit info for the current event:
  bool eventHasHit() { ensureCalc(); return m_hit; }
  //next methods only well defined when eventHasHit() returns true:
  double eventHitEDep() { ensureCalc(); return m_hitEDep; }
  double eventHitTime() { ensureCalc(); return m_hitTime; }
  void getEventHitPosition(double (&pos)[3]) { ensureCalc(); pos[0] = m_hitPos[0]; pos[1] = m_hitPos[1]; pos[2] = m_hitPos[2]; }
  const double* eventHitPosition() { ensureCalc(); return m_hitPos; }
  double eventHitPositionX() { ensureCalc(); return m_hitPos[0]; }
  double eventHitPositionY() { ensureCalc(); return m_hitPos[1]; }
  double eventHitPositionZ() { ensureCalc(); return m_hitPos[2]; }
  double eventHitWeight() { ensureCalc(); return m_hitWeight; }

  //convenience method which simply sums up all of the energy deposits,
  //regardless of thresholds and their location:
  double eventTotalEDep();

  const std::string& volumeName() { return m_volname; }
  ///////////////////////////////////////////////
  //todo: setter's (implies resetCalc())
  ///////////////////////////////////////////////

  void test();//Runs alg on test points

public:
  //callback triggered by the GriffDataReader each event:
  virtual void beginEvent(const GriffDataReader*) { resetCalc(); }
  //and triggered if we are deregistered (i.e. if the GriffDataReader goes out of scope before us)
  virtual void dereg(const GriffDataReader*) { m_dr = 0; }
private:
  void resetCalc() { m_needCalc=true; m_totalEDep = -1; }
  void ensureCalc() { if (m_needCalc) calc(); }
  void calcFromPoints();
  void calc();
  GriffDataReader * m_dr;
  GriffAnaUtils::SegmentIterator m_si;

  //settings:
  double m_cluster_outlier_threshold;
  double m_hit_edep_threshold;
  std::string m_volname;
  bool m_needCalc;

  //Results for current event:
  bool m_hit;
  double m_hitTime;
  double m_hitEDep;
  double m_hitWeight;
  double m_hitPos[3];
  double m_totalEDep;

  protected:
  //data structures used for calculations:
  struct Point {
    Point(const double* pos,double t,double e)
      : x(pos[0]), y(pos[1]), z(pos[2]), time(t), edep(e) {}
    Point(double xx, double yy, double zz,double t,double e)
      : x(xx), y(yy), z(zz), time(t), edep(e) {}
    double x, y, z;
    double time;
    double edep;
  };
private:
  std::vector<Point> m_points;
  struct PointSortByDist
  {
    PointSortByDist(const double (&refpoint)[3]) : x(refpoint[0]),y(refpoint[1]),z(refpoint[2]){}
    double x,y,z;
    double distsq( const Point& );
    bool operator() (const Point& p1, const Point& p2);
  };

  struct PointSortByTime
  {
    bool operator() (const Point& p1, const Point& p2) { return p1.time < p2.time; }
  };
  bool recalcCenter();//returns true if last point is an outlier. Updates m_hitEDep.

protected:
  //Turn the energy deposited over a segment into "Points"
  //(x,y,z,time,edep). The default implementation loops through the steps on the
  //segment and assumes that half of the energy deposition of each step occured
  //at each end of the step. It is the obligation of any alternative
  //implementation that exactly seg->eDep() is added in total, no more, no less.
  virtual void addPoints(std::vector<Point>& points,const GriffDataRead::Segment* seg);

};

#endif
