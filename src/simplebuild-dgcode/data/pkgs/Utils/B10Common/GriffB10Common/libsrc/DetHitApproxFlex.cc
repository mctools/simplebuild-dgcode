#include "GriffB10Common/DetHitApproxFlex.hh"
#include "Utils/ArrayMath.hh"

DetHitApproxFlex::DetHitApproxFlex( GriffDataReader* dr,
                                    double cluster_outlier_threshold,
                                    double hit_edep_threshold,
                                    const char * volname )
  : DetHitApproximation(dr,cluster_outlier_threshold,hit_edep_threshold,volname),
    m_segmentsOnly(false),
    m_n(2)
{
}

void DetHitApproxFlex::addPoints(std::vector<Point>& points,
                                 const GriffDataRead::Segment* seg) {
  if (m_segmentsOnly) {
      addp( points,seg->eDep(),
            seg->firstStep()->preGlobalArray(),seg->firstStep()->preTime(),
            seg->lastStep()->postGlobalArray(),seg->lastStep()->postTime() );
  } else {
    auto step = seg->stepBegin();
    auto stepE = seg->stepEnd();
    for (;step!=stepE;++step) {
      addp( points,step->eDep(),
            step->preGlobalArray(),step->preTime(),
            step->postGlobalArray(),step->postTime() );
    }
  }
}

void DetHitApproxFlex::addp( std::vector<Point>& points, double edep,
                             const double* p0, double t0,
                             const double* p1, double t1) const
{
  assert(m_n>=2);
  double v[3];
  double p[3];
  double e = edep/m_n;
  double c = 1.0/(m_n-1);
  Utils::subtract(p1,p0,v);
  for ( unsigned i = 0; i < m_n; ++i ) {
    if (i==0) {
      points.push_back(Point(p0,t0,e));
    } else if (i==m_n-1) {
      points.push_back(Point(p1,t1,e));
    } else {
      double s = i*c;
      Utils::mult(v,s,p);
      Utils::add(p0,p,p);
      points.push_back(Point(p,(t1-t0)*s+t0,e));
    }
  }
}

void DetHitApproxFlex::setPretendReduced(bool b)
{
  m_segmentsOnly = b;
}

void DetHitApproxFlex::setNPointsPerPath(unsigned n)
{
  m_n = n;
}

