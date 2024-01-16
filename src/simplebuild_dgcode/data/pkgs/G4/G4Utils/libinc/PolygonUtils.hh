#ifndef G4Utils_PolygonUtils_hh
#define G4Utils_PolygonUtils_hh

#include <vector>
#include <valarray>
#include "G4TwoVector.hh"

namespace G4Utils {

  typedef G4TwoVector PGUTwoVector;//should be very easy to migrate to other vector implementations
  typedef std::vector<PGUTwoVector> Polygon;//Polygons should be simple (i.e. not
                                           //self intersecting) and vertices in
                                           //anti-clockwise order. With at least
                                           //3 points of course (i.e. last and
                                           //first point NOT similar).

  double area(const Polygon&);//returned value will be <0 if points are clockwise rather than anti-clockwise

  //Grow a polygon by moving all edges outwards along their normals by a given
  //offset. The transition from one edge to the next is augmented with an extra
  //point (and thus 2 extra edges compared to the original shape) which is at
  //the position of the original point plus offset*0.5*(normal1+normal2) where
  //normal1 and normal2 are the normals of the two neighbouring edges. Resulting
  //polygon will have up to 3 times as many vertices as the original, and there
  //are no guarantee that the resulting polygon will not be self-intersecting if
  //the original polygon was concave:
  void growPolygon(const Polygon& orig,Polygon& result,double offset,bool smooth_corners=true);

  //Similar, but with offsets potentially differing for each face:
  void growPolygon(const Polygon& orig,Polygon& result,const std::vector<double>& offsets_thicknesses,bool smooth_corners=true);

  //TODO: verify that we can implement a shrinkPolygon simply by reversing the polygons order (clockwise/anti-clockwise)

  //returns intersection point between two non-parallel lines defined by points p1-p2 and p3-p4 respectively:
  PGUTwoVector lineIntersect(const PGUTwoVector& p1,const PGUTwoVector& p2,const PGUTwoVector& p3,const PGUTwoVector& p4);

  //convenience:
  void valarrays_2_Polygon(const std::valarray<double>& xvals,
                           const std::valarray<double>& yvals,
                           Polygon&);
  void polygon_2_valarrays(const Polygon&,
                           std::valarray<double>& xvals,
                           std::valarray<double>& yvals);
}


#endif
