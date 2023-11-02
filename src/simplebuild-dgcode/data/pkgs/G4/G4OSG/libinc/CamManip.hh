#ifndef G4OSG_CamManip_hh
#define G4OSG_CamManip_hh

#include <osgGA/TrackballManipulator>

//Extend the TrackballManipulator so we can:
//
// * Workaround an FPE in OSG 3.0.1
// * Add support for toggling orthographic/perspective view
// * Ensuring zoom works also in orthographic view in an intuitive way.
// * ...
//
// Note that we tie our manipulator directly to one camera (so we can also access the projection matrix for ortho zoom)

namespace G4OSG {

  class EventHandler;

  class CamManip : public osgGA::TrackballManipulator {

    typedef osgGA::TrackballManipulator inherited;

  public:

    CamManip(osg::Camera*);
    virtual ~CamManip(){}

    bool isOrthographic() const { return m_ortho; }
    bool isPerspective() const { return !m_ortho; }
    void setOrtho(bool o=true);
    void setPerspective(bool p=true) { setOrtho(!p); }
    void toggleOrtho() { setOrtho(!isOrthographic()); }

    void zoomToNode(osg::Node& node);

  protected:
    //Reimplemented to avoid divide-by-zero FPE in StandardManipulator::getThrowScale when eventTimeDelta=0
    virtual bool performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy );
    virtual bool performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy );
    virtual bool performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy );

    //Reimplemented to handle zoom in orthographic view:
    friend class ::G4OSG::EventHandler;
    virtual void zoomModel( const float dy, bool pushForwardIfNeeded = true );

  private:
    osg::Camera * m_cam;
    bool m_ortho;
    double m_orig_fovy;
  };

}

#endif
