#include "G4OSG/CamManip.hh"
#include <osg/ComputeBoundsVisitor>

G4OSG::CamManip::CamManip(osg::Camera*c)
  : osgGA::TrackballManipulator(),
    m_cam(c),
    m_ortho(false),
    m_orig_fovy(0)
{
  double dummy1,dummy2,dummy3;
  m_cam->getProjectionMatrixAsPerspective(m_orig_fovy,dummy1,dummy2,dummy3);
}

bool G4OSG::CamManip::performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy )
{
  if (eventTimeDelta<=0.0)
    return true;
  return inherited::performMovementLeftMouseButton(eventTimeDelta,dx,dy);
}

bool G4OSG::CamManip::performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy )
{
  if (eventTimeDelta<=0.0)
    return true;
  return inherited::performMovementMiddleMouseButton(eventTimeDelta,dx,dy);
}

bool G4OSG::CamManip::performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy )
{
  if (eventTimeDelta<=0.0)
    return true;
  return inherited::performMovementRightMouseButton(eventTimeDelta,dx,dy);
}

void G4OSG::CamManip::zoomModel( const float dy, bool pushForwardIfNeeded )
{
  if (isOrthographic() && dy != 0. ) {
    double scale = 1.0 + dy;
    double left,  right, bottom,  top, zNear,  zFar;
    m_cam->getProjectionMatrixAsOrtho(left,right,bottom,top,zNear,zFar);
    m_cam->setProjectionMatrixAsOrtho(scale*left,scale*right,
                                      scale*bottom,scale*top,
                                      zNear*scale,zFar*scale);
  }
  inherited::zoomModel( dy, pushForwardIfNeeded );
}

void G4OSG::CamManip::setOrtho(bool o)
{
  if (o==m_ortho)
    return;
  m_ortho = o;
  if (o) {
    //change to orthographic view
    double left,  right, bottom,  top, zNear,  zFar;
    m_cam->getProjectionMatrixAsFrustum(left,right,bottom,top,zNear,zFar);
    m_cam->setProjectionMatrixAsOrtho(left,right,bottom,top,zNear,zFar);
  } else {
    //change to perspective view
    double left,  right, bottom,  top, zNear,  zFar;
    m_cam->getProjectionMatrixAsOrtho(left,right,bottom,top,zNear,zFar);
    m_cam->setProjectionMatrixAsPerspective(m_orig_fovy,
                                            (right-left)/(top-bottom),
                                            zNear,zFar);

  }
}

void G4OSG::CamManip::zoomToNode(osg::Node& node)
{
  //Code heavily inspired by CameraManipulator::computeHomePosition and
  //StandardManipulator::home code.

  osg::BoundingSphere boundingSphere;

  osg::ComputeBoundsVisitor cbVisitor;
  node.accept(cbVisitor);
  osg::BoundingBox &bb = cbVisitor.getBoundingBox();
  if (bb.valid()) boundingSphere.expandBy(bb);
  else boundingSphere = node.getBound();

  osg::Vec3d oldeye,oldcenter, oldup;
  m_cam->getViewMatrixAsLookAt(oldeye,oldcenter,oldup/* lookDistance=1.0*/);

  // set dist to default
  double dist = 3.5 * boundingSphere.radius();

  // try to compute dist from frustrum
  double left,right,bottom,top,zNear,zFar;
  if (m_cam->getProjectionMatrixAsFrustum(left,right,bottom,top,zNear,zFar))
    {
      double vertical2 = fabs(right - left) / zNear / 2.;
      double horizontal2 = fabs(top - bottom) / zNear / 2.;
      double dim = horizontal2 < vertical2 ? horizontal2 : vertical2;
      double viewAngle = atan2(dim,1.);
      dist = boundingSphere.radius() / sin(viewAngle);
    }
  else
    {
      // try to compute dist from ortho
      if (m_cam->getProjectionMatrixAsOrtho(left,right,bottom,top,zNear,zFar))
        {
          dist = fabs(zFar - zNear) / 2.;
        }
    }

  setTransformation( boundingSphere.center() + (oldeye-oldcenter)*dist,
                     boundingSphere.center(),
                     oldup );

  flushMouseEventStack();
}
