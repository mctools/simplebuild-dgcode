#ifndef G4OSG_UserPoints_hh
#define G4OSG_UserPoints_hh

#include <osg/Vec3d>
#include <osg/Group>
#include <osg/ShapeDrawable>
#include <cassert>
#include <string>

namespace G4OSG {

  class Viewer;
  class VolHandle;

  class UserPoints {
  public:
    UserPoints(const std::string& datafile, Viewer*,osg::Group* sceneHook);
    ~UserPoints(){}

    void toggleVisible();

  private:
    bool m_isAttached;
    osg::ref_ptr<osg::Group> m_sceneHookExternal;
    osg::ref_ptr<osg::Group> m_sceneHook;
    Viewer* m_viewer;
    std::vector<osg::Vec3d> m_points;
    // unsigned nPointsDefined();
    // struct Point {
    //   Point() : defined(false) {}
    //   bool defined;
    //   osg::Vec3d point;
    //   osg::Vec3d normal;
    //   VolHandle * volHandle;
    //   osg::ref_ptr<osg::Node> node;
    //   osg::ref_ptr<osg::ShapeDrawable> shapedrawable;
    //   bool isOnCorner() const { return normal.length2()<10.0e-10; }
    // };
    // Point m_points[NPOINTS];
    void attach() { assert(!m_isAttached); m_sceneHookExternal->addChild(m_sceneHook); m_isAttached=true; }
    void detach() { assert(m_isAttached); m_sceneHookExternal->removeChild(m_sceneHook); m_isAttached=false; }
    // void clearPointInternal(POINT p);
    // void updateInfo();
    void createPoints();

    osg::Geode * createPoints(double ptsize, double r, double g, double b, double alpha);

  };
}

#endif
