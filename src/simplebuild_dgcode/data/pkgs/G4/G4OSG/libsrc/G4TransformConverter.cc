#include "G4OSG/G4TransformConverter.hh"
#include <cassert>

//In a scientific application We really do not want errors in transforms to
//propagate quickly:
#ifdef OSG_USE_FLOAT_MATRIX
#error OSG_USE_FLOAT_MATRIX is set which is normally not appropriate for scientific applications
#endif

namespace G4OSG {
  //Provide access to the protected data member _matrix:
  class MatrixTransformAccessor : public osg::MatrixTransform {
  public:
    osg::Matrix & accessMatrix() { return _matrix; }
  };
}

void G4OSG::setMatrix(osg::MatrixTransform&mt,const G4VPhysicalVolume&v)
{
  //    assert(false&&"toOSGMatrix not debugged yet");
  osg::Matrix & mat = static_cast<MatrixTransformAccessor*>(&mt)->accessMatrix();
  const G4ThreeVector& trans = v.GetTranslation();//inverse?
  //G4ThreeVector trans(.1*0.5,.2*0.5,.3*0.5);
  const G4RotationMatrix* rot = v.GetFrameRotation();//inverse?
  // rot = new G4RotationMatrix();
  // const_cast<G4RotationMatrix*>(rot)->rotateY(10*degree);
  if (rot) {
    mat.set( rot->xx(), rot->xy(), rot->xz(), 0,
             rot->yx(), rot->yy(), rot->yz(), 0,
             rot->zx(), rot->zy(), rot->zz(), 0,
             trans.x(), trans.y(), trans.z(), 1 );
  } else {
    mat.set( 1,         0,         0,         0,
             0,         1,         0,         0,
             0,         0,         1,         0,
             trans.x(), trans.y(), trans.z(), 1 );
  }

}

void G4OSG::setMatrix(osg::MatrixTransform& mt,const G4Transform3D& t)
{
  assert(false&&"setMatrix from G4Transform3D not debugged yet");
  osg::Matrix & mat = static_cast<MatrixTransformAccessor*>(&mt)->accessMatrix();
  mat.set(t.xx(), t.xy(), t.xz(), 0,
          t.yx(), t.yy(), t.yz(), 0,
          t.zx(), t.zy(), t.zz(), 0,
          t.dx(), t.dy(), t.dz(), 1);
}
