#include "G4OSG/RayIntersection.hh"
#include <vector>

bool G4OSG::rayIntersectsTriangle(const G4ThreeVector&p, const G4ThreeVector&d,
                                  const G4ThreeVector&v0, const G4ThreeVector&v1, const G4ThreeVector&v2,
                                  double& t) {
  //Finds "t" determining the intersection between:
  //ray : f(t) = p + d * t
  //triangle with vertices v0,v1,v2

  const double tolerance(1.0e-5);
  G4ThreeVector e1(v1-v0),e2(v2-v0);
  G4ThreeVector h = d.cross(e2);//cross product
  double a = e1.dot(h);

  if (a > -tolerance && a < tolerance)
    return false;

  double f = 1/a;
  G4ThreeVector ss=p-v0;
  double u = f * (ss.dot(h));

  if (u < 0.0 || u > 1.0)
    return false;

  G4ThreeVector q=ss.cross(e1);
  double v = f * (d.dot(q));

  if (v < 0.0 || u + v > 1.0)
    return false;

  // at this stage we can compute t to find out where
  // the intersection point is on the line
  t = f * (e2.dot(q));
  return true;
}

bool G4OSG::rayIntersectsPolyhedron(const HepPolyhedron& ph,
                                    const G4ThreeVector&p,
                                    const G4ThreeVector&d,
                                    G4ThreeVector&intersectionPoint,
                                    G4ThreeVector&intersectionNormal)
{
  G4int nFaces(ph.GetNoFacets());
  assert(nFaces>0);

  G4Point3D vertex[4];
  G4int edgeFlag[4];
  G4Normal3D normals[4];
  G4int nEdge;

  double t;

  std::vector<std::pair<double,G4int> > intersected_faces;//(t,iface)
  for (G4int iFace = 1; iFace <= nFaces; ++iFace) {
    ph.GetFacet(iFace, nEdge, vertex, edgeFlag,normals);
    assert(nEdge==3||nEdge==4);
    if (rayIntersectsTriangle(p, d,vertex[0],vertex[1],vertex[2],t)
        ||(nEdge==4&&rayIntersectsTriangle(p, d,vertex[0],vertex[2],vertex[3],t)))
      {
        intersected_faces.push_back(std::make_pair(t,iFace));
      }
  }
  if (intersected_faces.empty())
    return false;
  //we want the intersection with smallest t:
  std::sort(intersected_faces.begin(),intersected_faces.end());
  ph.GetFacet(intersected_faces[0].second, nEdge, vertex, edgeFlag,normals);
  intersectionPoint = p + intersected_faces[0].first * d;
  intersectionNormal = normals[0];//assuming flat faces
  return true;
}

