#include "G4Utils/PolygonUtils.hh"
#include <cassert>

double G4Utils::area(const Polygon& p)
{
  double area=0;
  int n = (int)p.size();
  assert(n>=3);
  for (int i=0;i<n;++i) {
    area+=p[i].x()*(p[(i+1)%n].y()-p[(i+n-1)%n].y());
  }
  return 0.5*area;
}

G4Utils::PGUTwoVector G4Utils::lineIntersect(const PGUTwoVector& p1,const PGUTwoVector& p2,const PGUTwoVector& p3,const PGUTwoVector& p4)
{
  const double x1m2(p1.x()-p2.x());
  const double y3m4(p3.y()-p4.y());
  const double y1m2(p1.y()-p2.y());
  const double x3m4(p3.x() - p4.x());
  double denom = x1m2*y3m4 - y1m2*x3m4;
  assert(denom);//not parallel
  const double x1y2my1x2(p1.x()*p2.y()-p1.y()*p2.x());
  const double x3y4my3x4(p3.x()*p4.y()-p3.y()*p4.x());
  denom = 1.0/denom;
  return PGUTwoVector( (x1y2my1x2*x3m4-x3y4my3x4*x1m2)*denom, (x1y2my1x2*y3m4-x3y4my3x4*y1m2)*denom);
}

void G4Utils::growPolygon(const Polygon& orig,Polygon& result,double offset,bool smooth_corners)
{
  std::vector<double> offsets;
  offsets.push_back(offset);
  growPolygon(orig,result,offsets,smooth_corners);
}

void G4Utils::growPolygon(const Polygon& orig,Polygon& result,const std::vector<double>& offsets,bool smooth_corners)
{
  const int n=orig.size();
  assert(n>=3);
  assert((int)offsets.size()==n||offsets.size()==1);
  result.clear();
  result.reserve(smooth_corners ? n*3 : n);//might overestimate slightly if orig not concave

  PGUTwoVector p(orig.at(n-1)), p_next(orig.at(0)), p_prev;
  PGUTwoVector normal_prev, normal((p_next-p).unit());
  normal.set(normal.y(),-normal.x());

  double offset_prev(0), offset;
  bool fixed_offset(offsets.size()==1);
  if (fixed_offset) {
    offset_prev = offset = offsets.at(0);
  } else {
    offset = offsets.at(n-1);
    assert(offset>=0);
  }

  const double tolerance(1.0e-6);

  for (int i=0; i<n;++i) {
    p_prev = p;
    p = p_next;
    p_next = orig.at((i+1)%n);

    normal_prev = normal;
    normal = (p_next-p).unit();
    normal.set(normal.y(),-normal.x());

    if (!fixed_offset) {
      offset_prev = offset;
      offset = offsets.at(i);
      assert(offset>=0);
    }

    //We are now at the i'th corner and we have the normal of the next and
    //previous edge as well as the coordinates of the 3 involved corners. Time
    //to get to work.

    double det = normal_prev.x()*normal.y()-normal_prev.y()*normal.x();

    bool almost_parallel = std::abs<double>(det)<tolerance;
    if ( almost_parallel || (smooth_corners && det > 0) ) {
      //Must add extra points for more rounded corner:
      if (!almost_parallel)
        result.push_back( p + offset_prev * normal_prev );
      result.push_back( p + 0.5 * (offset+offset_prev) * (normal+normal_prev).unit() );
      if (!almost_parallel)
        result.push_back( p + offset * normal );
    } else {
      //Translate the two edges outwards and put a corner at their
      //intersection:
      result.push_back(lineIntersect(p_next + offset * normal,
                                     p      + offset * normal,
                                     p      + offset_prev * normal_prev,
                                     p_prev + offset_prev * normal_prev));
    }
  }
}

void G4Utils::valarrays_2_Polygon(const std::valarray<double>& xvals,
                                  const std::valarray<double>& yvals,
                                  Polygon&p)
{
  const auto n = xvals.size();
  assert(yvals.size()==n);
  p.resize(n);
  for (unsigned i = 0; i<n; ++i)
    p[i].set(xvals[i],yvals[i]);
}

void G4Utils::polygon_2_valarrays(const Polygon&p,
                                  std::valarray<double>& xvals,
                                  std::valarray<double>& yvals)
{
  const auto n = p.size();
  xvals.resize(n);
  yvals.resize(n);
  for (unsigned i = 0; i<n; ++i) {
    const PGUTwoVector& v = p[i];
    xvals[i] = v.x();
    yvals[i] = v.y();
  }

}
