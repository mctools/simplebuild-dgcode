#include "G4OSG/Viewer.hh"
#include "G4OSG/VolHandle.hh"
#include "G4OSG/TrkHandle.hh"
#include "G4OSG/CamManip.hh"
#include "G4OSG/CoordAxes.hh"
#include "G4OSG/MeasurementPoints.hh"
#include "G4OSG/UserPoints.hh"

#include "G4RegionStore.hh"
#include "G4Region.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4OSG/BestUnit.hh"

#include <osg/DisplaySettings>
#include <osg/Multisample>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/CompositeViewer>

#include "Core/File.hh"

namespace G4Utils {
  //fixme: to G4Utils:
  const G4VPhysicalVolume * getWorldPV()
  {
    auto region_store = G4RegionStore::GetInstance();
    assert(region_store);
    for (auto itRegion = region_store->begin();itRegion!=region_store->end();++itRegion) {
      if ((*itRegion)->IsInMassGeometry())
        return (*itRegion)->GetWorldPhysical();
    }
    return 0;
  }
}

G4OSG::Viewer::~Viewer()
{
  delete m_measurementPoints;
  delete m_userPoints;
  delete m_coordAxes;
}

G4OSG::Viewer::Viewer(const char* griff_file,
                      bool show_geo,
                      const std::set<std::uint64_t>& griff_event_selection)
  : m_measurementPoints(0),
    m_userPoints(0),
    m_coordAxes(0),
    m_fullScreen(false),
    m_multiSampling(false),
    m_griff_file(griff_file?griff_file:""),
    m_griff_eventsToShow(griff_event_selection),
    m_show_geo(show_geo)
{
  init();
}

osgViewer::View* G4OSG::Viewer::createView( int x, int y, int w, int h, osg::Node* scene )
{
  auto newview = new osgViewer::View;
  newview->addEventHandler(m_eventHandler = new EventHandler(this));
  newview->setSceneData( scene );
  newview->setCameraManipulator( new CamManip(newview->getCamera()) );
  newview->setUpViewInWindow( x, y, w, h );
  return newview;
}

void G4OSG::Viewer::init()
{
  osg::ref_ptr<osg::Group> root = new osg::Group;

  ////////////////////////////////////////////////////////////////////////
  // Griff tracks:
  ////////////////////////////////////////////////////////////////////////

  if (!m_griff_file.empty()) {
    printf("Viewer: Will display the tracks in griff file \"%s\".\n",m_griff_file.c_str());
    printf("Viewer: The display of tracks is very rough and non-interactive for,\n");
    printf("Viewer: now, but the colours gives you a hint of their type:\n");
    printf("Viewer:\n");
    printf("Viewer:     Neutron  : Green\n");
    printf("Viewer:     Photon   : Yellow\n");
    printf("Viewer:     Electron : Blue\n");
    printf("Viewer:     Proton   : Red\n");
    printf("Viewer:     Pi+/Pi-  : Purple\n");
    printf("Viewer:     alpha    : Cyan\n");
    printf("Viewer:     Li7      : Orange\n");
    printf("Viewer:     Others   : White\n");
    printf("Viewer:\n");
    m_thcommon = new THCommon(this,m_griff_file.c_str(),m_griff_eventsToShow);
    m_thcommon->displayAllTracks();
    root->addChild(m_thcommon->sceneHook());
  }

  ////////////////////////////////////////////////////////////////////////
  // G4 geometry:
  ////////////////////////////////////////////////////////////////////////

  bool worldDisp(false);
  if (m_show_geo) {
    const G4VPhysicalVolume * pvWorld = G4Utils::getWorldPV();
    assert(pvWorld);
    m_vhcommon = new VHCommon(this,pvWorld);
    m_vhcommon->resetGeometry();
    worldDisp = m_vhcommon->volHandleWorld()->isDisplayed();
    if (m_vhcommon->volHandleWorld()->hasVisibleDaughtersRecursively())
      m_vhcommon->volHandleWorld()->hide();//hide during init, so initial view wont zoom out to world
    root->addChild(m_vhcommon->sceneHook());
  }

  ////////////////////////////////////////////////////////////////////////
  // Measurement points:
  ////////////////////////////////////////////////////////////////////////
  if (m_show_geo) {
    m_measurementPoints = new MeasurementPoints(this,root);
  }

  ////////////////////////////////////////////////////////////////////////
  // User points:
  ////////////////////////////////////////////////////////////////////////

  //Hidden option to display points. User must leave a file named
  //g4osg_points.txt in the current directory, with 3 columns of numbers: x y
  //and z coordinates of the points in millimeters.

  const std::string pointsfile = "g4osg_points.txt";
  if (Core::file_exists(pointsfile)) {
    m_userPoints = new UserPoints(pointsfile,this,root);
    m_userPoints->toggleVisible();
  }

  ////////////////////////////////////////////////////////////////////////
  // Coordinate axes:
  ////////////////////////////////////////////////////////////////////////
  m_coordAxes = new CoordAxes(this,root);

  double xmin, xmax, ymin, ymax, zmin, zmax;
  bool ok_extent(false);
  if (m_show_geo) {
    if (m_vhcommon->volHandleWorld()->nDaughters())
      ok_extent = m_vhcommon->volHandleWorld()->getExtentDaughters(xmin, xmax, ymin, ymax, zmin, zmax);
    else
      ok_extent = m_vhcommon->volHandleWorld()->getExtent(xmin, xmax, ymin, ymax, zmin, zmax);
    if (!ok_extent) {
      printf("Viewer: WARNING - Could not get extent of volumes to scale coordinate axes!\n");
    }
  } else if (hasGriffTracks()) {
    ok_extent = THCommon::getExtent(m_thcommon->trkHandles(), xmin, xmax, ymin, ymax, zmin, zmax);
    if (!ok_extent)
      printf("Viewer: WARNING - Could not get extent of tracks for coordinate axes scaling!\n");
  }
  if (ok_extent) {
    double tscale = (fabs(xmin)+fabs(xmax)+fabs(ymin)+fabs(ymax)+fabs(zmin)+fabs(zmax))/6.;
    tscale *= 0.4;
    unsigned i=0;
    while (m_coordAxes->getScale()>tscale && i<1e6) { ++i; m_coordAxes->scaleDown(); }
    i=0;
    while (m_coordAxes->getScale()<tscale && i<1e6) { ++i; m_coordAxes->scaleUp(); }
  }

  if (Viewer::envSet("G4OSG_SCALEAXES")) {
    int n = Viewer::envInt("G4OSG_SCALEAXES");
    int nabs = (n<0?-n:n);
    for (int i = 0; i < nabs; ++i)
      n>0 ? m_coordAxes->scaleUp() : m_coordAxes->scaleDown();
  }

  double coordscale_override = Viewer::envDbl("G4OSG_SCALEAXES_EXACT");
  if (coordscale_override>0)
    m_coordAxes->overrideScale(coordscale_override);

  //for outline FX (FIXME NOT ALWAYS SUPPORTED - we should remove outline mode again):
  //osg::DisplaySettings::instance()->setMinimumNumStencilBits(1);

  getScreenSize(m_screenSize.first,m_screenSize.second);
  int initial_x = 0.15*m_screenSize.first;
  int initial_y = 0.15*m_screenSize.second;
  int initial_width = 0.7*m_screenSize.first;
  int initial_height = 0.7*m_screenSize.second;

  //More than 16 might be supported, but I am not convinced it looks better.
  for (int testsamples = 16; ; testsamples /= 2) {
    osg::DisplaySettings::instance()->setNumMultiSamples( testsamples );
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(osg::DisplaySettings::instance());
    traits->x = initial_x ; traits->y = initial_y ; traits->width = initial_width ; traits->height = initial_height;
    traits->windowDecoration = true;
    auto nl = osg::getNotifyLevel();
    osg::setNotifyLevel(osg::WARN);
    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());//Leak (unclear why?)
    osg::setNotifyLevel(nl);
    if (gc.valid()) {
      //printf("Viewer: Graphics driver supports anti-aliasing (%i-times multisampling).\n",testsamples);
      break;
    } else if (testsamples==4) {
      printf("Viewer: WARNING - Graphics platform does NOT support anti-aliasing (no multisampling).\n");
      osg::DisplaySettings::instance()->setNumMultiSamples( 0 );
      break;
    }
  }
  m_view = createView(initial_x, initial_y,initial_width,initial_height,root.get());
#ifdef OSG_USE_FLOAT_PLANE
  printf("Viewer: WARNING - OpenSceneGraph was compiled with OSG_USE_FLOAT_PLANE, expected reduced precision.\n");
#endif
  auto compviewer = new osgViewer::CompositeViewer;//fixme: should not use a composite viewer
  m_viewer = compviewer;
  compviewer->addView( m_view );

  compviewer->setKeyEventSetsDone(0);//no quit

  m_viewer->setUpThreading();

  // clear stencil buffer... (for outline FX)
  unsigned int clearMask = m_view->getCamera()->getClearMask();
  m_view->getCamera()->setClearMask(clearMask | GL_STENCIL_BUFFER_BIT);
  m_view->getCamera()->setClearStencil(0);

  //NOTE: Keep this next value in synch with m_bgdColor=0  value in EventHandler.cc!!!!:
  m_view->getCamera()->setClearColor(osg::Vec4(0.4,0.4,0.4,1));


  if (Viewer::envOption("G4OSG_BGWHITE")) {
    //like this for now, should actually change m_bgdColor state in evt handler...
    m_view->getCamera()->setClearColor(osg::Vec4(1.0,1.0,1.0,1));
  }
  if (Viewer::envOption("G4OSG_BGBLACK")) {
    //like this for now, should actually change m_bgdColor state in evt handler...
    m_view->getCamera()->setClearColor(osg::Vec4(0.0,0.0,0.0,1));
  }

  //Disable small-feature-culling as it is mainly a source of confusion:
  osg::CullStack::CullingMode cullingMode = m_view->getCamera()->getCullingMode();
  cullingMode &= ~(osg::CullStack::SMALL_FEATURE_CULLING);
  m_view->getCamera()->setCullingMode( cullingMode );

  m_hud = new HUD(m_view,compviewer,1280,1024);//Note that we do *NOT* base this on actual
  //screen/window dimensions (to avoid overflow
  //issues on different screens)
  m_hud->setText(HUD::TOP_LEFT,"Help: CTRL-H");
  m_hud->setText(HUD::TOP_CENTER,"CoolNameHere");
  m_hud->setText(HUD::TOP_RIGHT,"Quit: CTRL-Q");

  osgViewer::ViewerBase::Windows windows;
  m_viewer->getWindows(windows);
  assert(windows.size()==1);
  auto thewindow = (*windows.begin());
  thewindow->setWindowName("CoolNameHere");

  auto ss = m_view->getCamera()->getOrCreateStateSet();
  ss->setAttributeAndModes(new osg::Multisample, osg::StateAttribute::ON);
  ss->setMode(GL_MULTISAMPLE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

  if (osg::DisplaySettings::instance()->getNumMultiSamples()>0)
    toggleMultiSampling();

  if (Viewer::envOption("G4OSG_NOWORLDWIREFRAME")&&m_vhcommon->volHandleWorld()->nDaughters())
    worldDisp=false;//Prevent wireframe on initially expanded world

  if (worldDisp)
    m_vhcommon->volHandleWorld()->display();

  m_coordAxes->toggleVisible();//Start visible
  if (hasManyVisibleGriffTracks()) {
    std::cout<<"Viewer: WARNING - High number of tracks. Disabling track-info-on-mouseover"<<std::endl;
    std::cout<<"Viewer:           except when user holds down either the SHIFT or CTRL key."<<std::endl;
  }
  std::cout<<"Viewer: Expanding world volume as wireframe"<<std::endl;
  std::cout<<"Viewer: Showing axes with length: "<<G4OSG::BestUnit(m_coordAxes->getScale(), "Length")<<std::endl;//todo: visual feedback
}

int G4OSG::Viewer::run()
{
  if ( ! ( (m_thcommon&&!m_thcommon->trkHandles().empty()) || m_show_geo ) ) {
    printf("Viewer ERROR: Aborting since there is nothing to display\n");
    return 1;
  }
  int res = m_viewer->run();
  m_viewer->stopThreading();//important to not get spurious segfaults due to delayed cleanup
  return res;
}

osgViewer::GraphicsWindow* G4OSG::Viewer::window()
{
  osgViewer::ViewerBase::Windows windows;
  m_viewer->getWindows(windows);
  assert(windows.size()==1);
  return (*windows.begin());
}

void G4OSG::Viewer::toggleFullScreen() {

  //inspired by code in osgViewer/ViewerEventHandlers.cpp

  // sleep to allow any viewer rendering threads to complete before we
  // resize the window
  m_fullScreen = !m_fullScreen;
  OpenThreads::Thread::microSleep(100000);
  auto w = window();

  if (m_fullScreen) {
    //going fullscreen
    w->setWindowDecoration(false);
    w->setWindowRectangle(0, 0, m_screenSize.first, m_screenSize.second);
  } else {
    //going back to window
    w->setWindowDecoration(true);
    int dim_x = 0.15*m_screenSize.first;
    int dim_y = 0.15*m_screenSize.second;
    int dim_width = 0.7*m_screenSize.first;
    int dim_height = 0.7*m_screenSize.second;
    w->setWindowRectangle(dim_x, dim_y, dim_width, dim_height);
  }
  w->grabFocusIfPointerInWindow();
}

void G4OSG::Viewer::getScreenSize(unsigned&w,unsigned& h) const {
  w=h=1024;
  osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
  assert(wsi);
  // if (!wsi) {
  //   osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
  //   return;
  // }
  wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0),w,h);
}

void G4OSG::Viewer::toggleMultiSampling()
{
  if (osg::DisplaySettings::instance()->getNumMultiSamples()==0) {
    printf("Viewer: ERROR - Anti-aliasing not supported by graphics platform.\n");
    return;
  }

  auto ss = m_view->getCamera()->getOrCreateStateSet();
  m_multiSampling=!m_multiSampling;

  m_viewer->stopThreading();

  if (m_multiSampling) {
    //enable multisampling
    ss->setMode(GL_MULTISAMPLE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    printf("Viewer: Enabling multisampling (anti-aliasing)\n");
  } else {
    //disable multisampling
    ss->setMode(GL_MULTISAMPLE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    printf("Viewer: Disabling multisampling (anti-aliasing)\n");
  }

  m_viewer->startThreading();

}
#include <cstdlib>//for getenv
bool G4OSG::Viewer::envOption(const char * option)
{
  const char * val = getenv(option);
  if (!val)
    return false;
  std::string v(val);
  if (v.empty()||v=="0"||v=="NO"||v=="no"||v=="off"||v=="OFF"||v=="n")
    return false;
  return true;
}

std::string G4OSG::Viewer::envStr(const char * option)
{
  const char * val = getenv(option);
  if (!val)
    return std::string();
  return std::string(val);
}

bool G4OSG::Viewer::envSet(const char * option)
{
  return getenv(option)!=0;
}

int G4OSG::Viewer::envInt(const char * option, int val_if_not_set)
{
  const char * val = getenv(option);
  if (!val)
    return val_if_not_set;
  return std::stoi(val);
}

double G4OSG::Viewer::envDbl(const char * option, double val_if_not_set)
{
  const char * val = getenv(option);
  if (!val)
    return val_if_not_set;
  return std::stod(val);
}
