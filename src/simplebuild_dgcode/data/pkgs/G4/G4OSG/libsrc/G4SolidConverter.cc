#include "G4OSG/G4SolidConverter.hh"

#include <osg/Geometry>
//...

#include "G4Polyhedron.hh"
#include "G4VSolid.hh"
#include "G4Box.hh"
//#include "G4Sphere.hh"
//...

#include <cassert>

osg::ref_ptr<osg::Geode> G4OSG::g4solid2Geode(const G4VSolid* solid) {
  auto drawable = g4solid2Geometry(solid);
  osg::ref_ptr<osg::Geode> geode = new osg::Geode;
  geode->addDrawable( drawable.release() );
  geode->setName(solid->GetName());
  return geode;
}

osg::ref_ptr<osg::Geometry> G4OSG::g4solid2Geometry(const G4VSolid* solid)
{
  //For consistent results we do not use the standard Shape, but rather use G4
  //polyhedronisation and osg::Geometry objects for all shapes.

  G4Polyhedron* ph = solid->GetPolyhedron();

  if (!ph) {
    printf("Viewer ERROR: Could not get polyhedron from the following solid:\n");
    std::cout<<*solid<<std::endl;
    throw std::runtime_error("G4OSG::g4solid2Geometry error : GetPolyhedron on"
                             " solid returns null ptr (polyhedronisation not available?). ");
  }

  G4int nFaces(ph->GetNoFacets());
  assert(nFaces>0);

  osg::ref_ptr<osg::Geometry> osgGeom = new osg::Geometry();
  osg::ref_ptr<osg::Vec3Array> osgVertices = new osg::Vec3Array;//todo: double precision??
  osg::ref_ptr<osg::Vec3Array> osgNormals = new osg::Vec3Array;
  osgVertices->reserve(nFaces*4);//slight overallocation in case of triangles, but not worse on average than blind push_back.
  osgNormals->reserve(nFaces*4);

  G4Point3D vertex[4];
  G4int edgeFlag[4];
  G4Normal3D normals[4];
  G4int nEdge;

  //TODO: recycle these arrays?:
  //TODO: Vec3Array instead?
  osg::ref_ptr<osg::Vec3Array> triangularVertices = new osg::Vec3Array;
  osg::ref_ptr<osg::Vec3Array> triangularNormals = new osg::Vec3Array;

  std::vector<G4int> triangularFaces;
  for (G4int iFace = 1; iFace <= nFaces; ++iFace) {
    ph->GetFacet(iFace, nEdge, vertex,edgeFlag,normals);
    assert(nEdge==3||nEdge==4);
    for (G4int iEdge=0;iEdge<nEdge;++iEdge) {
      if (nEdge==4) {
        osgVertices->push_back(osg::Vec3(vertex[iEdge].x(),vertex[iEdge].y(),vertex[iEdge].z()));
        osgNormals->push_back(osg::Vec3(normals[iEdge].x(),normals[iEdge].y(),normals[iEdge].z()));
      } else {
        triangularVertices->push_back(osg::Vec3(vertex[iEdge].x(),vertex[iEdge].y(),vertex[iEdge].z()));
        triangularNormals->push_back(osg::Vec3(normals[iEdge].x(),normals[iEdge].y(),normals[iEdge].z()));
      }
    }
  }

  unsigned nquads(osgVertices->size());
  if (!triangularVertices->empty()) {
    osgVertices->insert(osgVertices->end(), triangularVertices->begin(), triangularVertices->end());
    osgNormals->insert(osgNormals->end(), triangularNormals->begin(), triangularNormals->end());
  }

  osgGeom->setVertexArray(osgVertices.get());
  osgGeom->setNormalArray(osgNormals.get());
  osgGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);//todo: consider BIND_PER_PRIMITIVE
  if (nquads)
    osgGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,nquads));
  if (!triangularVertices->empty())
    osgGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,nquads,triangularVertices->size()));

  osgGeom->setName(solid->GetName());
  return osgGeom;
}
