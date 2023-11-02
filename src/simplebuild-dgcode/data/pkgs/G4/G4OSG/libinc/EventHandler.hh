#ifndef G4OSG_EventHandler_hh
#define G4OSG_EventHandler_hh

#include <osgGA/GUIEventHandler>
#include <osgViewer/View>
#include <osg/Geode>
#include <osg/Timer>
#include <set>

namespace G4OSG {
  class Viewer;
  class VolHandle;
  class TrkHandle;
  class ObjectHolder;
}

namespace G4OSG {
  class EventHandler : public osgGA::GUIEventHandler {
  public:
    EventHandler(Viewer*);
  protected:
    using osgGA::GUIEventHandler::handle;
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*);
  private:
    VolHandle* pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea,
                    osg::Vec3d& pickedPoint, osg::Vec3d& pickedNormal);
    TrkHandle* pickTrack(osgViewer::View* view, const osgGA::GUIEventAdapter& ea,
                         osg::Vec3d& pickedPoint, unsigned& pickedStep);
    void updateHoverInfo(osgViewer::View* view, const osgGA::GUIEventAdapter& ea);
    void toggleHelp(bool force_on=false);
    bool isHelp() const;
    void initHelp();
    //osg::ref_ptr<osg::Geode> createPoint(double x,double y,double z);
    Viewer* m_viewer;
    std::set<int> m_keys_pressed;
    //double m_orig_fovy;
    std::string m_helptxt_left;
    std::string m_helptxt_right;
    unsigned m_bgdColor;
    double m_push_x;
    double m_push_y;
    enum MOUSE_ACTION { CENTER_VIEW_ON_POINT, CENTER_VIEW_ON_CORNER, ORIENT_TO_SURFACE,
                        ZOOM_TO_OBJECT, GEO_EXPAND_TO_DAUGHTERS, GEO_CONTRACT_TO_MOTHER,
                        GEO_ZAP_VOLUME, GEO_WIREFRAME_EXPAND,
                        //DISABLE GEO_SELECT_BY_LOGVOL, GEO_SELECT_BY_MATERIAL,
                        GEO_SELECT_VOL,
                        //DISABLE GEO_SELECT_BY_LOGVOL_MULTI, GEO_SELECT_BY_MATERIAL_MULTI, GEO_SELECT_VOL_MULTI,
                        MEASPOINT_MOVE_RED_TO_CORNER, MEASPOINT_MOVE_RED,
                        MEASPOINT_MOVE_GREEN_TO_CORNER, MEASPOINT_MOVE_GREEN,
                        MEASPOINT_MOVE_BLUE_TO_CORNER, MEASPOINT_MOVE_BLUE,
                        COORD_AXES_PLACE_ON_VOL_CORNER, COORD_AXES_PLACE_ON_VOL, NONE };
    osg::Vec3d m_home_eye;
    osg::Vec3d m_home_center;
    osg::Vec3d m_home_up;
    osg::Timer m_select_timer;
    VolHandle * m_hoverVH;
    std::pair<TrkHandle *,unsigned> m_hoverTH;
    void clearHoverInfo();
  };
}

#endif
