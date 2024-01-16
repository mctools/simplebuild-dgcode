#include "GriffB10Common/DetHitApproximation.hh"
#include "GriffAnaUtils/SegmentFilter_Volume.hh"
#include "GriffAnaUtils/SegmentFilter_EnergyDeposition.hh"
#include <stdexcept>
#include <algorithm>
#include <set>

DetHitApproximation::DetHitApproximation(GriffDataReader*dr,
                                         double cluster_outlier_threshold,
                                         double hit_edep_threshold,
                                         const char * volname)
  : m_dr(dr),
    m_si(dr),
    m_cluster_outlier_threshold(cluster_outlier_threshold),
    m_hit_edep_threshold(hit_edep_threshold),
    m_volname(volname),
    m_needCalc(true),
    m_hit(false),
    m_hitTime(0.0),
    m_hitEDep(0.0),
    m_hitWeight(0.0)
{
  assert(hit_edep_threshold>0);
  m_hitPos[0] = m_hitPos[1] = m_hitPos[2] = 0.0;
  dr->registerBeginEventCallBack(this);
  m_si.addFilter(new GriffAnaUtils::SegmentFilter_Volume(volname));
  m_si.addFilter(new GriffAnaUtils::SegmentFilter_EnergyDeposition(0*Units::eV));
}

DetHitApproximation::~DetHitApproximation()
{
  if (m_dr)
    m_dr->deregisterBeginEventCallBack(this);
}

void DetHitApproximation::calc()
{
  assert(m_dr);
  assert(m_needCalc);
  m_needCalc=false;
  m_points.clear();

  if (m_dr->eventStorageMode()==GriffFormat::Format::MODE_MINIMAL)
    throw std::runtime_error("DetHitApproximation does not work with Griff files in MINIMAL mode");

  //Collect points from steps:
  m_si.reset();
  double edeptot(0);
  std::set<double> weights;
  while (auto seg =  m_si.next()) {
    edeptot += seg->eDep();
    auto nbef = m_points.size();
    addPoints(m_points,seg);
    if (nbef!=m_points.size())
      weights.insert(seg->getTrack()->weight());
  }
  m_hitWeight = 0.0;
  if (weights.size()>1)
    throw std::runtime_error("Depositions from particles with different weights not supported.");
  if (weights.size()==1)
    m_hitWeight = *(weights.begin());
  if (m_totalEDep<0)
    m_totalEDep = edeptot;
  calcFromPoints();
}

void DetHitApproximation::addPoints(std::vector<Point>& points,const GriffDataRead::Segment* seg) {
  auto step = seg->stepBegin();
  auto stepE = seg->stepEnd();
  for (;step!=stepE;++step) {
    points.push_back(Point(step->preGlobalArray(),step->preTime(),step->eDep()*0.5));
    points.push_back(Point(step->postGlobalArray(),step->postTime(),step->eDep()*0.5));
  }
}

void DetHitApproximation::calcFromPoints()
{
  //m_points has already been filled, proceed calculations from there.
  m_needCalc=false;
  m_hit=false;
  m_hitTime = m_hitEDep = m_hitPos[0] = m_hitPos[1] = m_hitPos[2] = 0.0;

  if (m_points.empty())
    return;

  //Resursively find weighed (by edep) center and throw away the furthest point
  //if above dist threshold:
  while(recalcCenter())
    m_points.pop_back();

  //Was there a hit?
  m_hit = (m_hitEDep>=m_hit_edep_threshold);

  if (m_hit) {
    //Find time of hit => sort hits by time and take the time of the last of the
    //earlier hits which is enough to exceed the edep threshold.
    PointSortByTime timesort;
    std::sort(m_points.begin(),m_points.end(),timesort);
    m_hitTime = m_points.back().time;
    double edep_sum(0);
    auto itE=m_points.end();
    for (auto it=m_points.begin();it!=itE;++it) {
      edep_sum += it->edep;
      if (edep_sum>=m_hit_edep_threshold) {
        m_hitTime = it->time;
        break;
      }
    }
  }
}

inline double DetHitApproximation::PointSortByDist::distsq( const Point& p )
{
  double dx = p.x-x;
  double dy = p.y-y;
  double dz = p.z-z;
  return dx*dx+dy*dy+dz*dz;
}

inline bool DetHitApproximation::PointSortByDist::operator() (const Point& p1, const Point& p2)
{
  return distsq(p1)<distsq(p2);
}

bool DetHitApproximation::recalcCenter()
{
  //Find center of points, total edep, and sort by distance to center.
   m_hitEDep = 0.0;
   m_hitPos[0] = m_hitPos[1] = m_hitPos[2] = 0.0;
   auto itE=m_points.end();
   for (auto it=m_points.begin();it!=itE;++it) {
     m_hitEDep += it->edep;
     m_hitPos[0] += it->x*it->edep;
     m_hitPos[1] += it->y*it->edep;
     m_hitPos[2] += it->z*it->edep;
   }
   m_hitPos[0] /= m_hitEDep;
   m_hitPos[1] /= m_hitEDep;
   m_hitPos[2] /= m_hitEDep;
   PointSortByDist distsort(m_hitPos);
   std::sort(m_points.begin(),m_points.end(),distsort);
   return distsort.distsq(m_points.back())>m_cluster_outlier_threshold*m_cluster_outlier_threshold;
}

void DetHitApproximation::test()
{
  resetCalc();
  //Perform calculations based on some test points, use unit thresholds
  const double cthr = m_cluster_outlier_threshold;
  const double ethr = m_hit_edep_threshold;
  m_cluster_outlier_threshold = 1.0;
  m_hit_edep_threshold = 1.0;

  //Fake hits: center should be 0.025 0.025 0.025 0.025 and time 10.0
  double p1[3] = { 0, 0, 0 };
  double p2[3] = { 0.1, 0, 0 };
  double p3[3] = { 0, 0.1, 0 };
  double p4[3] = { 0, 0, 0.1 };
  double p5[3] = { 0, 0, 999.0 };

  printf("TEST1 ====================================\n");
  m_points.clear();
  m_points.push_back(Point(p1,10.0,1.01));
  m_points.push_back(Point(p2,11.0,1.01));
  m_points.push_back(Point(p3,12.0,1.01));
  m_points.push_back(Point(p4,13.0,1.01));
  calcFromPoints();
  printf("Hit: %s\n",(eventHasHit()?"yes":"no"));
  if (eventHasHit()) {
    printf("Edep: %g\n",eventHitEDep());
    printf("Time: %g\n",eventHitTime());
    printf("Center: (%g,%g,%g)\n",eventHitPositionX(),eventHitPositionY(),eventHitPositionZ());
  }
  printf("TEST2 ====================================\n");
  m_points.clear();
  m_points.push_back(Point(p1,10.0,1.01));
  m_points.push_back(Point(p2,11.0,1.01));
  m_points.push_back(Point(p3,12.0,1.01));
  m_points.push_back(Point(p4,13.0,1.01));
  m_points.push_back(Point(p5,8.0,3.01));
  calcFromPoints();
  printf("Hit: %s\n",(eventHasHit()?"yes":"no"));
  if (eventHasHit()) {
    printf("Edep: %g\n",eventHitEDep());
    printf("Time: %g\n",eventHitTime());
    printf("Center: (%g,%g,%g)\n",eventHitPositionX(),eventHitPositionY(),eventHitPositionZ());
  }
  printf("TEST3 ====================================\n");
  m_points.clear();
  m_points.push_back(Point(p1,10.0,0.2*1.01));
  m_points.push_back(Point(p1,14.0,0.8*1.01));
  m_points.push_back(Point(p2,11.0,1.01));
  m_points.push_back(Point(p3,12.0,1.01));
  m_points.push_back(Point(p4,13.0,1.01));
  m_points.push_back(Point(p5,8.0,3.01));
  calcFromPoints();
  printf("Hit: %s\n",(eventHasHit()?"yes":"no"));
  if (eventHasHit()) {
    printf("Edep: %g\n",eventHitEDep());
    printf("Time: %g\n",eventHitTime());
    printf("Center: (%g,%g,%g)\n",eventHitPositionX(),eventHitPositionY(),eventHitPositionZ());
  }
  printf("TEST4 ====================================\n");
  m_points.clear();
  m_points.push_back(Point(p1,10.0,0.8));
  m_points.push_back(Point(p5,10.0,0.8));
  calcFromPoints();
  printf("Hit: %s\n",(eventHasHit()?"yes":"no"));
  if (eventHasHit()) {
    printf("Edep: %g\n",eventHitEDep());
    printf("Time: %g\n",eventHitTime());
    printf("Center: (%g,%g,%g)\n",eventHitPositionX(),eventHitPositionY(),eventHitPositionZ());
  }
  printf("TEST5 ====================================\n");
  m_points.clear();
  m_points.push_back(Point(p4,12.0,1.8));
  calcFromPoints();
  printf("Hit: %s\n",(eventHasHit()?"yes":"no"));
  if (eventHasHit()) {
    printf("Edep: %g\n",eventHitEDep());
    printf("Time: %g\n",eventHitTime());
    printf("Center: (%g,%g,%g)\n",eventHitPositionX(),eventHitPositionY(),eventHitPositionZ());
  }
  printf("TEST6 ====================================\n");
  m_points.clear();
  calcFromPoints();
  printf("Hit: %s\n",(eventHasHit()?"yes":"no"));
  if (eventHasHit()) {
    printf("Edep: %g\n",eventHitEDep());
    printf("Time: %g\n",eventHitTime());
    printf("Center: (%g,%g,%g)\n",eventHitPositionX(),eventHitPositionY(),eventHitPositionZ());
  }
  printf("TESTEND ==================================\n");

  m_cluster_outlier_threshold = cthr;
  m_hit_edep_threshold = ethr;
  resetCalc();
}

double DetHitApproximation::eventTotalEDep()
{
  assert(m_dr);
  if (m_totalEDep>=0)
    return m_totalEDep;
  m_totalEDep = 0;
  m_si.reset();
  while (auto seg =  m_si.next())
    m_totalEDep += seg->eDep();
  return m_totalEDep;
}
