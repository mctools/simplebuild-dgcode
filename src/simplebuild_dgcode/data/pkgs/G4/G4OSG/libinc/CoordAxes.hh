#ifndef G4OSG_CoordAxes_hh
#define G4OSG_CoordAxes_hh

#include <osg/Group>
#include <osg/MatrixTransform>
#include "Core/Types.hh"

namespace G4OSG {
  class Viewer;
  class CoordAxes {
  public:
    CoordAxes(Viewer*,osg::Group* sceneHook);
    ~CoordAxes();

    void setPos(const osg::Vec3d&, bool makeVisible=false);
    //void setScale(double, bool makeVisible=false);
    void scaleUp(bool makeVisible=false);
    void scaleDown(bool makeVisible=false);
    double getScale() const;
    void toggleVisible();
    void overrideScale(double, bool makeVisible=false);

  private:
    void scaleChanged(bool makeVisible);
    Viewer * m_viewer;
    osg::ref_ptr<osg::Group> m_sceneHook;
    osg::ref_ptr<osg::MatrixTransform> m_trf;
    osg::Vec3d m_pos;
    unsigned m_scalemod;
    std::uint64_t m_scale_base_nanometer;
    //double m_scale;
    bool m_visible;
    void updateTrf();
    osg::ref_ptr<osg::Node> createAxesModel();
  };
}

#endif
