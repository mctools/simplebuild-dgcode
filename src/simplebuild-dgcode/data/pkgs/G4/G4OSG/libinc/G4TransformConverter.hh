#ifndef G4OSG_G4TransformConverter_hh
#define G4OSG_G4TransformConverter_hh

#include "G4Transform3D.hh"
#include "G4VPhysicalVolume.hh"
#include <osg/MatrixTransform>

namespace G4OSG {

  //Set the matrix on an existing matrix transform directly based on G4 objects:
  void setMatrix(osg::MatrixTransform&,const G4VPhysicalVolume&);
  void setMatrix(osg::MatrixTransform&,const G4Transform3D&);

  //Create new matrix transforms based on G4 objects:
  osg::MatrixTransform * createMatrixTransform(const G4VPhysicalVolume&v);
  osg::MatrixTransform * createMatrixTransform(const G4Transform3D&t);
}

#include "G4OSG/G4TransformConverter.icc"

#endif
