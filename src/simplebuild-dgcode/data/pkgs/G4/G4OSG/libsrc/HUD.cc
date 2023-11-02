#include "G4OSG/HUD.hh"
#include <osgText/Text>
#include <osg/Version>
#include <cassert>
#include "Core/FindData.hh"//NB: Is this our only dependency on other packages?
#include <cstdlib>//getenv

G4OSG::HUD::HUD(osgViewer::View*view ,osgViewer::ViewerBase*viewer, double width, double height)
  : m_view(view), m_viewer(viewer), m_cam(0),
    m_width(width), m_height(height),
    m_hudVisible(false)
{
  m_textMargin = (m_width+m_height)*0.5*0.01;
  m_boxMargin = m_textMargin*0.3;
  initCamera();
  initTexts();
  initBoxes();
  setHUDVisibility(true);
}

G4OSG::HUD::~HUD()
{
}

void G4OSG::HUD::setHUDVisibility(bool v)
{
  if (m_hudVisible==v)
    return;
  m_viewer->stopThreading();//fixme: G4OSG::Viewer should have premod/postmod
                            //methods (with integer incrementing behind)... or
                            //an UpdateGuard guard(viewer); interface. Use in volhandle hide/display as well.
  m_hudVisible=v;
  if (v) {
    m_cam->addChild(m_textGeode);
    m_cam->addChild(m_boxGeode);
  } else {
    m_cam->removeChild(m_textGeode);
    m_cam->removeChild(m_boxGeode);
  }
  m_viewer->startThreading();
}

void G4OSG::HUD::initCamera()
{
  // create a HUD as slave camera attached to the master view.
  osgViewer::ViewerBase::Windows windows;
  m_viewer->getWindows(windows);
  assert(!windows.empty());

  // create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
  m_cam = new osg::Camera;

  // set the projection matrix
  m_cam->setProjectionMatrix(osg::Matrix::ortho2D(0,m_width,0,m_height));

  // set the view matrix
  m_cam->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
  m_cam->setViewMatrix(osg::Matrix::identity());

  // only clear the depth buffer
  m_cam->setClearMask(GL_DEPTH_BUFFER_BIT);

  // draw subgraph after main camera view.
  m_cam->setRenderOrder(osg::Camera::POST_RENDER);

  // we don't want the camera to grab event focus from the viewers main camera(s).
  m_cam->setAllowEventFocus(false);

  //Content simply scales upon window resizing:
  m_cam->setProjectionResizePolicy(osg::Camera::FIXED);

  m_cam->setGraphicsContext(windows[0]);
  m_cam->setViewport(0,0,windows[0]->getTraits()->width, windows[0]->getTraits()->height);
  m_view->addSlave(m_cam, false);
}

void G4OSG::HUD::initTexts()
{
  m_textGeode = new osg::Geode;
  m_textGeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
  if (m_hudVisible)
    m_cam->addChild(m_textGeode);

  //For consistency we always use the same TTF file included with our code
  //RobotoMono_wght_.ttf (an APACHE-2.0 licensed font):
  std::string fontfile = Core::findData("G4OSG","RobotoMono_wght_.ttf");
  auto fontfile_override = std::getenv("DGCODE_G4OSG_FONTFILE_OVERRIDE");
  if ( fontfile_override )
    fontfile = fontfile_override;

  for (unsigned i=0;i<=(unsigned)LAST_TEXTPOS;++i) {
    m_texts[i] = new osgText::Text;
    m_texts[i]->setFont(fontfile);
    //m_texts[i]->setCharacterSize(10);//fixme: constructor option
  }

  m_texts[TOP_LEFT]->setAlignment(osgText::TextBase::LEFT_TOP);
  m_texts[TOP_LEFT]->setPosition(osg::Vec3(m_textMargin,m_height-m_textMargin,-1.5f));
  m_texts[TOP_CENTER]->setAlignment(osgText::TextBase::CENTER_TOP);
  m_texts[TOP_CENTER]->setPosition(osg::Vec3(m_width*0.5,m_height-m_textMargin,-1.5f));
  m_texts[TOP_RIGHT]->setAlignment(osgText::TextBase::RIGHT_TOP);
  m_texts[TOP_RIGHT]->setPosition(osg::Vec3(m_width-m_textMargin,m_height-m_textMargin,-1.5f));
  m_texts[BOTTOM_LEFT]->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
  m_texts[BOTTOM_LEFT]->setPosition(osg::Vec3(m_textMargin,m_textMargin,-2.5f));
  m_texts[BOTTOM_CENTER]->setAlignment(osgText::TextBase::CENTER_BOTTOM_BASE_LINE);
  m_texts[BOTTOM_CENTER]->setPosition(osg::Vec3(m_width*0.5,m_textMargin,2.5f));
  m_texts[BOTTOM_RIGHT]->setAlignment(osgText::TextBase::RIGHT_BOTTOM_BASE_LINE);
  m_texts[BOTTOM_RIGHT]->setPosition(osg::Vec3(m_width-m_textMargin,m_textMargin,-2.5f));

  m_texts[BOTTOM_CENTER]->setAlignment(osgText::TextBase::CENTER_BOTTOM_BASE_LINE);

  double small(0.7*m_texts[TOP_LEFT]->getCharacterHeight());
  //double verysmall(0.85*small);
  m_texts[BOX_LEFT]->setCharacterSize(small);
  m_texts[BOX_LEFT]->setAlignment(osgText::TextBase::LEFT_CENTER);
  m_texts[BOX_LEFT]->setPosition(osg::Vec3(m_textMargin,0.5*m_height,0.0f));

  m_texts[BOX_RIGHT]->setCharacterSize(small);
  m_texts[BOX_RIGHT]->setAlignment(osgText::TextBase::LEFT_CENTER);
  m_texts[BOX_RIGHT]->setPosition(osg::Vec3(0.5*m_width,0.5*m_height,0.0f));

  m_texts[BOX_CENTER]->setCharacterSize(small);
  m_texts[BOX_CENTER]->setAlignment(osgText::TextBase::LEFT_CENTER);
  m_texts[BOX_CENTER]->setPosition(osg::Vec3(0.5*m_width,0.5*m_height,-1.0f));

  m_texts[BOX_CENTER_COLUMN1]->setCharacterSize(small);
  m_texts[BOX_CENTER_COLUMN1]->setAlignment(osgText::TextBase::CENTER_CENTER);
  m_texts[BOX_CENTER_COLUMN1]->setPosition(osg::Vec3(0.5*m_width,0.5*m_height,0.0f));

  m_texts[BOX_CENTER_COLUMN2]->setCharacterSize(small);
  m_texts[BOX_CENTER_COLUMN2]->setAlignment(osgText::TextBase::CENTER_CENTER);
  m_texts[BOX_CENTER_COLUMN2]->setPosition(osg::Vec3(0.5*m_width,0.5*m_height,0.0f));

  m_texts[BOTTOM_LEFT]->setCharacterSize(small);
  m_texts[BOTTOM_CENTER]->setCharacterSize(small);
  m_texts[BOTTOM_RIGHT]->setCharacterSize(small);

}

void G4OSG::HUD::initBoxes()
{
  m_boxGeode = new osg::Geode;
  m_boxGeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
  createBox(m_boxTop,-2);
  createBox(m_boxBottom,-3);
  createBox(m_boxCenterCols,-1,osg::Vec4(0.0,0.0,0.0,0.85));
  createBox(m_boxLeft,  -4,osg::Vec4(0.5,0.5,0.4,0.6));
  createBox(m_boxCenter,-4,osg::Vec4(0.5,0.5,0.5,0.9));

  if (m_hudVisible)
    m_cam->addChild(m_boxGeode);
}

void G4OSG::HUD::setText(TEXTPOS p, const std::string& s )
{
  if (m_textStrs[p]==s)
    return;
  m_viewer->stopThreading();
  bool prev_empty = m_textStrs[p].empty();
  m_textStrs[p] = s;

  if (true)//UTF8
    m_texts[p]->setText(osgText::String(s,osgText::String::ENCODING_UTF8));
  else//No UTF8
    m_texts[p]->setText(s);

  if (prev_empty)
    m_textGeode->addDrawable(m_texts[p]);
  else if (s.empty())
    m_textGeode->removeDrawable(m_texts[p]);

  if (p==BOTTOM_LEFT||p==BOTTOM_CENTER||p==BOTTOM_RIGHT) {
    updateBoxBottom();
  } else if (p==TOP_LEFT||p==TOP_CENTER||p==TOP_RIGHT) {
    updateBoxTop();
  } else if (p==BOX_CENTER_COLUMN1||p==BOX_CENTER_COLUMN2) {
    updateBoxAndTextCenterCol();
  } else if (p==BOX_LEFT) {
    updateBoxLeft();
  } else if (p==BOX_CENTER) {
    updateBoxAndTextCenter();
  } else if (p==BOX_RIGHT) {
    double w,h;
    if (getTextDim(BOX_RIGHT, w,h))
      m_texts[BOX_RIGHT]->setPosition(osg::Vec3(m_width-w-m_textMargin,0.5*m_height,0.0f));
  }
  m_viewer->startThreading();
}


bool G4OSG::HUD::getTextDim(TEXTPOS tp, double& width, double& height) const
{
  if (m_textStrs[tp].empty()) {
    width = height = 0;
    return false;
  }
  double x0, y0, x1, y1;
  getTextBounds(m_texts[tp], x0,  y0,  x1,  y1);
  width = x1-x0;
  height = y1-y0;
  return true;
}

void G4OSG::HUD::getTextBounds(osgText::Text*txt,double& xmin, double& ymin, double& xmax, double& ymax) const
{
#if !OSG_VERSION_LESS_THAN(3,3,2)
  auto bb = txt->computeBoundingBox();
#else
  auto bb = txt->getBound();
#endif
  xmin = bb.xMin();
  xmax = bb.xMax();
  ymin = bb.yMin();
  ymax = bb.yMax();
}

void G4OSG::HUD::updateBoxLeft()
{
  double w,h;
  if (!getTextDim(BOX_LEFT, w,h)) {
    //no text, hide box
    if (m_boxLeft.attached) {
      m_boxGeode->removeDrawable(m_boxLeft.geom);
      m_boxLeft.attached = false;
    }
    return;
  }
  //update box:

  (*(m_boxLeft.vertices))[0].x() = m_boxMargin;
  (*(m_boxLeft.vertices))[1].x() = m_boxMargin;
  (*(m_boxLeft.vertices))[2].x() = m_width - m_boxMargin;
  (*(m_boxLeft.vertices))[3].x() = m_width - m_boxMargin;
  (*(m_boxLeft.vertices))[1].y() = m_height*0.5 -0.5*h - m_boxMargin;
  (*(m_boxLeft.vertices))[2].y() = m_height*0.5 -0.5*h - m_boxMargin;
  (*(m_boxLeft.vertices))[3].y() = m_height*0.5 +0.5*h + m_boxMargin;
  (*(m_boxLeft.vertices))[0].y() = m_height*0.5 +0.5*h + m_boxMargin;

  //show:
  if (!m_boxLeft.attached) {
    m_boxGeode->addDrawable(m_boxLeft.geom);
    m_boxLeft.attached = true;
  }


}

void G4OSG::HUD::updateBoxAndTextCenter()
{
  double w,h;
  if (!getTextDim(BOX_CENTER, w,h)) {
    //no text, hide box
    if (m_boxCenter.attached) {
      m_boxGeode->removeDrawable(m_boxCenter.geom);
      m_boxCenter.attached = false;
    }
    return;
  }
  //update text:
  double zorder=-3.0;
  m_texts[BOX_CENTER]->setPosition(osg::Vec3(0.5*m_width - 0.5*w,0.5*m_height,zorder));

  //update box:
  double margin(3*m_boxMargin);//want bigger here...

  (*(m_boxCenter.vertices))[0].x() = 0.5*m_width-0.5*w-margin;
  (*(m_boxCenter.vertices))[1].x() = 0.5*m_width-0.5*w-margin;
  (*(m_boxCenter.vertices))[2].x() = 0.5*m_width+0.5*w+margin;
  (*(m_boxCenter.vertices))[3].x() = 0.5*m_width+0.5*w+margin;
  (*(m_boxCenter.vertices))[1].y() = m_height*0.5 -0.5*h - margin;
  (*(m_boxCenter.vertices))[2].y() = m_height*0.5 -0.5*h - margin;
  (*(m_boxCenter.vertices))[3].y() = m_height*0.5 +0.5*h + margin;
  (*(m_boxCenter.vertices))[0].y() = m_height*0.5 +0.5*h + margin;

  //show:
  if (!m_boxCenter.attached) {
    m_boxGeode->addDrawable(m_boxCenter.geom);
    m_boxCenter.attached = true;
  }


}

void G4OSG::HUD::updateBoxTop()
{
  double ymin = m_height+1;

  double w,h;
  if (getTextDim(TOP_LEFT, w,h)) ymin = std::min<double>(ymin,m_height-h-m_textMargin);
  if (getTextDim(TOP_CENTER, w,h)) ymin = std::min<double>(ymin,m_height-h-m_textMargin);
  if (getTextDim(TOP_RIGHT, w,h)) ymin = std::min<double>(ymin,m_height-h-m_textMargin);

  if (ymin>m_height) {
    //should not show box
    if (m_boxTop.attached) {
      m_boxGeode->removeDrawable(m_boxTop.geom);
      m_boxTop.attached = false;
    }
    return;
  }
  //update box:

  (*(m_boxTop.vertices))[1].y() = ymin - m_boxMargin;
  (*(m_boxTop.vertices))[2].y() = ymin - m_boxMargin;
  //show:
  if (!m_boxTop.attached) {
    m_boxGeode->addDrawable(m_boxTop.geom);
    m_boxTop.attached = true;
  }
}

void G4OSG::HUD::updateBoxBottom()
{
  double ymax = -1;

  double w,h;
  if (getTextDim(BOTTOM_LEFT, w,h)) ymax = std::max<double>(ymax,h+m_textMargin);
  if (getTextDim(BOTTOM_CENTER, w,h)) ymax = std::max<double>(ymax,h+m_textMargin);
  if (getTextDim(BOTTOM_RIGHT, w,h)) ymax = std::max<double>(ymax,h+m_textMargin);

  if (ymax<0) {
    //should not show box
    if (m_boxBottom.attached) {
      m_boxGeode->removeDrawable(m_boxBottom.geom);
      m_boxBottom.attached = false;
    }
    return;
  }

  //update box:

  (*(m_boxBottom.vertices))[0].y() = ymax + m_boxMargin;
  (*(m_boxBottom.vertices))[3].y() = ymax + m_boxMargin;
  //show:
  if (!m_boxBottom.attached) {
    m_boxGeode->addDrawable(m_boxBottom.geom);
    m_boxBottom.attached = true;
  }
}

void G4OSG::HUD::updateBoxAndTextCenterCol()
{
  double w1(0),w2(0),h1(0),h2(0);
  double left(0.5*m_width),right(0.5*m_width);
  osg::ref_ptr<osgText::Text> text1 = m_texts[BOX_CENTER_COLUMN1];
  osg::ref_ptr<osgText::Text> text2 = m_texts[BOX_CENTER_COLUMN2];

  double tdw,tdh;
  if (getTextDim(BOX_CENTER_COLUMN1, tdw,tdh)) { w1=tdw; h1=tdh; }
  if (getTextDim(BOX_CENTER_COLUMN2, tdw,tdh)) { w2=tdw; h2=tdh; }

  if (w1>0&&w2==0) {
    //just column 1
    text1->setAlignment(osgText::TextBase::CENTER_CENTER);
    text1->setPosition(osg::Vec3(m_width*0.5,0.5*m_height,0.0f));
    left-=0.5*w1;//-m_boxMargin;
    right+=0.5*w1;//+m_boxMargin;
  } else if (w1==0&&w2>0) {
    //just column 2
    text2->setAlignment(osgText::TextBase::CENTER_CENTER);
    text2->setPosition(osg::Vec3(m_width*0.5,0.5*m_height,0.0f));
    left-=0.5*w2;//-m_boxMargin;
    right+=0.5*w2;//+m_boxMargin;
  } else if (w1>0&&w2>0) {
    //both columns
    double colspace = 3*m_textMargin;
    double w = w1+w2+colspace;
    left -= 0.5*w;// - m_boxMargin;
    right += 0.5*w;// + m_boxMargin;
    text1->setAlignment(osgText::TextBase::LEFT_CENTER);
    text2->setAlignment(osgText::TextBase::LEFT_CENTER);
    text1->setPosition(osg::Vec3(m_width*0.5-0.5*w,0.5*m_height,0.0f));
    text2->setPosition(osg::Vec3(m_width*0.5-0.5*w+w1+colspace,0.5*m_height,0.0f));
  }

  //now to the box:

  if (w1==0&&w2==0) {
    //should not show box
    if (m_boxCenterCols.attached) {
      m_boxGeode->removeDrawable(m_boxCenterCols.geom);
      m_boxCenterCols.attached = false;
    }
    return;
  }
  double margin(m_boxMargin*5);
  left  -= margin;
  right += margin;
  double hmax = std::max<double>(h1,h2);
  double top=m_height*0.5+0.5*hmax+margin;
  double bottom=m_height*0.5-0.5*hmax-margin;
  (*(m_boxCenterCols.vertices))[0].x() = left;
  (*(m_boxCenterCols.vertices))[0].y() = top;
  (*(m_boxCenterCols.vertices))[1].x() = left;
  (*(m_boxCenterCols.vertices))[1].y() = bottom;
  (*(m_boxCenterCols.vertices))[2].x() = right;
  (*(m_boxCenterCols.vertices))[2].y() = bottom;
  (*(m_boxCenterCols.vertices))[3].x() = right;
  (*(m_boxCenterCols.vertices))[3].y() = top;

  //show:
  if (!m_boxCenterCols.attached) {
    m_boxGeode->addDrawable(m_boxCenterCols.geom);
    m_boxCenterCols.attached = true;
  }
}


void G4OSG::HUD::createBox(Box& box, double zdepth, const osg::Vec4& colour )
{
  osg::Geometry* geom = new osg::Geometry;
  // geom->setDataVariance( osg::Object::DYNAMIC );
  // vertices->setDataVariance( osg::Object::DYNAMIC );
  // normals->setDataVariance( osg::Object::STATIC );
  // colors->setDataVariance( osg::Object::STATIC );

  osg::Vec3Array* vertices = new osg::Vec3Array;
  vertices->reserve(4);
  //default: cover the whole view
  vertices->push_back(osg::Vec3(0,m_height,zdepth));
  vertices->push_back(osg::Vec3(0,0,zdepth));
  vertices->push_back(osg::Vec3(m_width,0,zdepth));
  vertices->push_back(osg::Vec3(m_width,m_height,zdepth));
  geom->setVertexArray(vertices);

  osg::Vec3Array* normals = new osg::Vec3Array;
  normals->push_back(osg::Vec3(0.0f,0.0f,-1.0f));
  geom->setNormalArray(normals);
  geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

  osg::Vec4Array* colors = new osg::Vec4Array;
  //colors->push_back(osg::Vec4(1.0f,1.0,0.8f,0.2f));
  colors->push_back(colour);
  geom->setColorArray(colors);
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);
  geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
  geom->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
  geom->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
  //geom->getOrCreateStateSet()->setAttribute(new osg::PolygonOffset(1.0f,1.0f),osg::StateAttribute::ON);

  box.geom = geom;
  box.vertices = vertices;
  box.attached = false;
}
