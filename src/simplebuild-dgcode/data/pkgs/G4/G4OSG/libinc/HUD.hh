#ifndef G4OSG_HUD_hh
#define G4OSG_HUD_hh

#include <osgViewer/View>
#include <osgViewer/ViewerBase>
#include <osgText/Text>
#include <string>

namespace G4OSG {

  class HUD : public osg::Referenced {
  public:
    HUD(osgViewer::View*,osgViewer::ViewerBase*, double width, double height);
    virtual ~HUD();

    void toggleHUDVisibility() { setHUDVisibility(!getHUDVisibility()); }
    void setHUDVisibility(bool v=true);
    bool getHUDVisibility() const { return m_hudVisible; }

    enum TEXTPOS { TOP_LEFT=0, TOP_CENTER, TOP_RIGHT,
                   BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT,
                   BOX_LEFT, BOX_RIGHT, BOX_CENTER,
                   //For help menu:
                   BOX_CENTER_COLUMN1, BOX_CENTER_COLUMN2,
                   LAST_TEXTPOS=BOX_CENTER_COLUMN2 };

    void setText(TEXTPOS p, const std::string& s );
    const std::string& getText(TEXTPOS p) const { return m_textStrs[p]; }
    void clearText(TEXTPOS p) { setText(p,""); }

  private:
    osgViewer::View* m_view;
    osgViewer::ViewerBase* m_viewer;
    osg::Camera* m_cam;//HUD camera
    const double m_width;
    const double m_height;
    double m_textMargin;
    double m_boxMargin;
    osg::ref_ptr<osg::Geode> m_textGeode;
    osg::ref_ptr<osgText::Text> m_texts[LAST_TEXTPOS+1];
    std::string m_textStrs[LAST_TEXTPOS+1];

    osg::ref_ptr<osg::Geode> m_boxGeode;
    struct Box {
      Box() : attached(false) {}
      osg::ref_ptr<osg::Geometry> geom;
      osg::ref_ptr<osg::Vec3Array> vertices;
      bool attached;
    };
    Box m_boxTop;
    Box m_boxBottom;
    Box m_boxCenterCols;
    Box m_boxLeft;
    Box m_boxCenter;

    bool m_hudVisible;

    void initCamera();
    void initTexts();
    void initBoxes();

    void updateBoxTop();
    void updateBoxBottom();
    void updateBoxAndTextCenterCol();
    void updateBoxLeft();
    void updateBoxAndTextCenter();
    void createBox(Box&, double zdepth, const osg::Vec4& colour = osg::Vec4(0.0f,0.0,0.0f,0.5f));
    void getTextBounds(osgText::Text*txt,double& xmin, double& ymin, double& xmax, double& ymax) const;
    bool getTextDim(TEXTPOS, double& width, double& height) const;
  };

}

#endif
