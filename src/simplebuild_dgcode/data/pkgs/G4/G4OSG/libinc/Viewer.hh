#ifndef G4OSG_Viewer_hh
#define G4OSG_Viewer_hh

#include "G4OSG/HUD.hh"
#include "G4OSG/VHCommon.hh"
#include "G4OSG/THCommon.hh"
#include "G4OSG/EventHandler.hh"
#include <cassert>
#include <cstdint>
#include <set>

namespace G4OSG {
  class MeasurementPoints;
  class UserPoints;
  class CoordAxes;

  class Viewer {
  public:
    Viewer(const char * griff_file = 0, bool show_geo = true,
           const std::set<std::uint64_t>& griff_event_selection = std::set<std::uint64_t>());
    ~Viewer();
    int run();

    HUD& hud() { assert(m_hud.valid()); return *m_hud; }
    EventHandler& eventHandler() { assert(m_eventHandler.valid()); return *m_eventHandler; }//fixme rename pickhandler
    VHCommon& vhCommon() { assert(m_vhcommon.valid()); return *m_vhcommon; }
    THCommon& thCommon() { assert(m_thcommon.valid()); return *m_thcommon; }
    osgViewer::View& view() { assert(m_view.valid()); return *m_view; }
    bool viewerValid() { return m_viewer.valid(); }
    void safeStartThreading() { if (m_viewer.valid()) m_viewer->startThreading(); }
    void safeStopThreading() { if (m_viewer.valid()) m_viewer->stopThreading(); }

    osgViewer::ViewerBase& viewer() { assert(m_viewer.valid()); return *m_viewer; }
    MeasurementPoints& measurementPoints() { return *m_measurementPoints; }
    //UserPoints& userPoints() { return *m_userPoints; }
    CoordAxes& coordAxes() { return *m_coordAxes; }

    void toggleFullScreen();
    bool isFullScreen() const { return m_fullScreen; }
    osgViewer::GraphicsWindow* window();
    void toggleMultiSampling();
    static bool envSet(const char * option);
    static bool envOption(const char * option);
    static std::string envStr(const char * option);
    static int envInt(const char * option, int val_if_not_set = 0);
    static double envDbl(const char * option, double val_if_not_set = 0);

    bool hasGeo() const { return m_show_geo; }
    bool hasGriffTracks() const { return m_thcommon && !m_thcommon->trkHandles().empty(); }
    bool hasManyVisibleGriffTracks() const { return m_thcommon && m_thcommon->trkHandles().size()>10000; }//fixme: should only count *visible* tracks

  private:
    osg::ref_ptr<HUD> m_hud;
    osg::ref_ptr<EventHandler> m_eventHandler;
    osg::ref_ptr<VHCommon> m_vhcommon;
    osg::ref_ptr<THCommon> m_thcommon;
    osg::ref_ptr<osgViewer::View> m_view;
    osg::ref_ptr<osgViewer::ViewerBase> m_viewer;
    MeasurementPoints * m_measurementPoints;
    UserPoints * m_userPoints;
    CoordAxes * m_coordAxes;

    bool m_fullScreen;
    std::pair<unsigned,unsigned> m_screenSize;//(width,height)
    bool m_multiSampling;
    osgViewer::View* createView( int x, int y, int w, int h, osg::Node* );
    void getScreenSize(unsigned&w,unsigned& h) const;
    void init();
    //Event data:
    std::string m_griff_file;
    std::set<std::uint64_t> m_griff_eventsToShow;
    bool m_show_geo;
  };
}

#endif
