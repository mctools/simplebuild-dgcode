#ifndef G4OSG_MeasurementPoints_hh
#define G4OSG_MeasurementPoints_hh

#include <osg/Vec3d>
#include <osg/Group>
#include <osg/ShapeDrawable>
#include <cassert>

namespace G4OSG {

  class Viewer;
  class VolHandle;

  class MeasurementPoints {
  public:
    MeasurementPoints(Viewer*,osg::Group* sceneHook);
    ~MeasurementPoints(){}
    enum POINT { RED=0, GREEN=1, BLUE=2, NPOINTS=3 };

    void toggleVisible();

    void clearPoint(POINT);

    //setting any point will also cause the points to become visible (set normal when point is on a surface rather than a corner)
    void setPoint(POINT,VolHandle* vh,const osg::Vec3d&, const osg::Vec3d& normal = osg::Vec3d(0,0,0));

  private:
    bool m_isAttached;
    osg::ref_ptr<osg::Group> m_sceneHookExternal;
    osg::ref_ptr<osg::Group> m_sceneHook;
    Viewer* m_viewer;
    unsigned nPointsDefined();
    struct Point {
      Point() : defined(false) {}
      bool defined;
      osg::Vec3d point;
      osg::Vec3d normal;
      VolHandle * volHandle;
      osg::ref_ptr<osg::Node> node;
      osg::ref_ptr<osg::ShapeDrawable> shapedrawable;
      bool isOnCorner() const { return normal.length2()<10.0e-10; }
    };
    Point m_points[NPOINTS];
    void attach() { assert(!m_isAttached); m_sceneHookExternal->addChild(m_sceneHook); m_isAttached=true; }
    void detach() { assert(m_isAttached); m_sceneHookExternal->removeChild(m_sceneHook); m_isAttached=false; }
    void clearPointInternal(POINT p);
    void updateInfo();
  };
}

#endif
