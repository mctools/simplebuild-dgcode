#ifndef G4OSG_RayIntersection_hh
#define G4OSG_RayIntersection_hh

#include "G4ThreeVector.hh"
#include "HepPolyhedron.h"
#include <cassert>

namespace G4OSG {

  //if the ray defined by p+d*t intersections the triangle (v0,v1,v2), return true and the value of t:
  bool rayIntersectsTriangle(const G4ThreeVector&p, const G4ThreeVector&d,
                             const G4ThreeVector&v0, const G4ThreeVector&v1, const G4ThreeVector&v2,
                             double& t);

  //If the ray intersects the polyhedron, return true and the (point,normal) for the intersection with smallest value of t (from p+d*t):
  bool rayIntersectsPolyhedron(const HepPolyhedron& ph,
                               const G4ThreeVector&p,
                               const G4ThreeVector&d,
                               G4ThreeVector&intersectionPoint,
                               G4ThreeVector&intersectionNormal);
}

#endif
