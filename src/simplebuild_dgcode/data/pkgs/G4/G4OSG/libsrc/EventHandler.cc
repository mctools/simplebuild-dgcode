#include "G4OSG/EventHandler.hh"
#include "G4OSG/ObjectHolder.hh"
#include "G4OSG/CamManip.hh"
#include "G4OSG/VolHandle.hh"
#include "G4OSG/TrkHandle.hh"
#include "G4OSG/Viewer.hh"
#include "G4OSG/MeasurementPoints.hh"
#include "G4OSG/CoordAxes.hh"
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Version>
#include "G4OSG/BestUnit.hh"

G4OSG::EventHandler::EventHandler(Viewer*viewer)
  : osgGA::GUIEventHandler(),
    m_viewer(viewer),
    //m_orig_fovy(-1),
    m_bgdColor(0),
    m_push_x(-1e99),
    m_push_y(-1e99),
    m_hoverVH(0),
    m_hoverTH(std::make_pair<TrkHandle *,unsigned>(0,0))

{
  initHelp();
}

// osg::ref_ptr<osg::Geode> G4OSG::EventHandler::createPoint(double x,double y,double z)
// {
//   //Fixme: this is a sphere for now - need an actual point
//   if (true) {
//     osg::ref_ptr<osg::ShapeDrawable> testshape = new osg::ShapeDrawable;
//     testshape->setShape( new osg::Sphere(osg::Vec3(x,y,z), 10.0));
//     osg::ref_ptr<osg::Geode> testgeode = new osg::Geode;
//     testshape->setColor( osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f) );
//     testgeode->addDrawable( testshape.get() );
//     return testgeode;
//   }
// }

bool G4OSG::EventHandler::handle(const osgGA::GUIEventAdapter& ea,
                                 osgGA::GUIActionAdapter&aa,
                                 osg::Object*, osg::NodeVisitor*)
{
  if (ea.getEventType()==osgGA::GUIEventAdapter::FRAME)
    return false;//ignore this right away for efficiency

  osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
  auto camManip = dynamic_cast<G4OSG::CamManip*>(view->getCameraManipulator());
  auto cam = view->getCamera();
  assert(view&&cam&&camManip);

  //Key presses, making sure letters work with shift/caps-lock
  const bool shift_down = ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_SHIFT;
  bool ctrl_down = ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL;
  auto key = ea.getKey();
  assert('a'>'A');
  if (key>='A'&&key<='Z')
    key += ('a'-'A');

  //todo: mac-key cmd should work like ctrl.

  //workaround weird bug where pressing (and holding) CTRL and then a letter
  //leads to key in the range 1..26 rather than 'a'..'z':
  //This bug was confirmed in osg 3.0.1 on Linux.
  if (key>=1&&key<=26&&ctrl_down)
    key+='a'-1;

  // if (key==osgGA::GUIEventAdapter::KEY_Control_L||key==osgGA::GUIEventAdapter::KEY_Control_R) {
  //   //stuff like CTRL-Q should work also when Q is pressed before CTRL (SHIFT
  //   //seems to be a true modkey, not firing events by itself).
  //   if (m_keys_pressed.size()==1)
  //     key=*(m_keys_pressed.begin());
  //   ctrl_down=true;
  // }

  //Mouse clicks: At most one can be interpreted (but we allow left+right=middle simulation)
  bool left_click = ea.getButton()&osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
  bool right_click = ea.getButton()&osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
  bool middle_click = ea.getButton()&osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;

  if (left_click&&right_click) {
    //emulate middle click
    middle_click=true;
    left_click=right_click=false;
  }
  if (left_click)
    right_click=middle_click=false;
  if (middle_click)
    left_click=right_click=false;

  VHCommon* vhCommon = (m_viewer->hasGeo() ? & ( m_viewer->vhCommon() ) : 0);//null if geo not enabled
  switch(ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::MOVE):
      if (vhCommon) {
        //Clear selection, but only if a bit of time elapsed, to prevent jitter
        //during clicking to immediately close the info window:
        if (m_select_timer.time_s()>0.5)
          vhCommon->clearAllSelectedHandles();
      }
      if (!left_click&&!right_click)
        updateHoverInfo(view,ea);
      return false;

      //In case we need to catch window close events:
      // case(osgGA::GUIEventAdapter::CLOSE_WINDOW):
      //   return false;
      // case(osgGA::GUIEventAdapter::QUIT_APPLICATION):
      //  return false;
    case(osgGA::GUIEventAdapter::RESIZE):
      //This should work:
      //  updateHoverInfo(view,ea);
      //but doesn't always seem to update correctly, so we clear instead:
      clearHoverInfo();
      return false;
    case(osgGA::GUIEventAdapter::KEYDOWN):
      if ((key==osgGA::GUIEventAdapter::KEY_H&&ctrl_down)||key==osgGA::GUIEventAdapter::KEY_F1) {
        if (!m_viewer->hud().getHUDVisibility()) {
          //must show help!
          toggleHelp(true);
          m_viewer->hud().setHUDVisibility(true);
          updateHoverInfo(view,ea);
        } else {
          toggleHelp();
        }
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_Delete||key==osgGA::GUIEventAdapter::KEY_BackSpace) {
        //KEY_KP_Delete
        if (vhCommon) {
          vhCommon->unzapPrevious();
          updateHoverInfo(view,ea);
        }
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_Space) {
        if (m_home_up.length2()<1.0e-10) {
          camManip->getHomePosition(m_home_eye, m_home_center, m_home_up);
          assert(m_home_up.length2()>1.0e-10);
        }
        if (camManip->isOrthographic()) {
          //shake ortho out of a bad state:
          camManip->toggleOrtho();
          camManip->home(ea,aa);
          camManip->toggleOrtho();
        }
        //camManip->resetMatrix();
        bool vis_world(false);
        if (vhCommon) {
          auto world = vhCommon->volHandleWorld();
          if (world->isDisplayed()&&world->hasVisibleDaughtersRecursively()) {
            vis_world=true;
            world->hide();//hide briefly so home zoom wont include world
          }
        } else {
          //todo: something about tracks?
        }
        if (m_home_up.length2()>1.0e-10) {
          camManip->setHomePosition(m_home_eye, m_home_center, m_home_up, false/*autoComputeHomePosition*/);
        }
        camManip->home(ea,aa);
        //camManip->resetMatrix();
        if (vis_world&&vhCommon)
          vhCommon->volHandleWorld()->display();
        updateHoverInfo(view,ea);
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_1||key==osgGA::GUIEventAdapter::KEY_2||key==osgGA::GUIEventAdapter::KEY_3) {
        if (m_home_up.length2()<1.0e-10) {
          camManip->getHomePosition(m_home_eye, m_home_center, m_home_up);
          assert(m_home_up.length2()>1.0e-10);
        }
        if (camManip->isOrthographic()) {
          //attempt to shake ortho out of a bad state:
          camManip->toggleOrtho();
          camManip->home(ea,aa);
          camManip->toggleOrtho();
        }
        //Trigger standard home computation, to take advantage of the resulting
        //eye-center distance (hide world if needed):
        osg::Vec3d eye, center, up;
        bool world_vis(false);
        VolHandle * world = vhCommon?vhCommon->volHandleWorld():0;
        if ( world && world->isDisplayed() && world->hasVisibleDaughtersRecursively() ) {
          world_vis = true;
          world->hide();
        }
        bool toggleOrtho = false;
        if (camManip->isOrthographic()) {
          camManip->toggleOrtho();
          toggleOrtho = true;
        }
        camManip->computeHomePosition(cam,true/*useBoundingBox - for higher precision*/);
        camManip->getHomePosition(eye, center, up);
        double ecdist = (eye-center).length();
        if (toggleOrtho)
          camManip->toggleOrtho();
        if (world_vis)
          world->display();
        //Replace with our own up, center and eye direction:
        if (key==osgGA::GUIEventAdapter::KEY_1) { eye.set(1,0,0); up.set(0,0,1); }
        else if (key==osgGA::GUIEventAdapter::KEY_2) { eye.set(0,1,0); up.set(1,0,0); }
        else if (key==osgGA::GUIEventAdapter::KEY_3) { eye.set(0,0,1); up.set(0,1,0); }
        center.set(0,0,0);
        eye = eye*ecdist + center;
        //apply:
        camManip->setHomePosition(eye, center, up, false/*autoComputeHomePosition*/);
        camManip->home(ea,aa);
        // if (toggleOrtho&&false)
        //   camManip->toggleOrtho();
        updateHoverInfo(view,ea);
        return true;
      } else if (ctrl_down&&key==osgGA::GUIEventAdapter::KEY_Z) {
        if (vhCommon) {
          vhCommon->unzapAll();
          updateHoverInfo(view,ea);
        }
        return true;
      } else if (ctrl_down&&key==osgGA::GUIEventAdapter::KEY_V) {
        if (vhCommon)
          vhCommon->resetGeometry();
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_E) {
        if (vhCommon&&!vhCommon->selectedHandles().empty())
          vhCommon->requestExtendedInfo();
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_0) {
        m_viewer->coordAxes().setPos(osg::Vec3d(0,0,0),true);
        return true;
      } else if (ctrl_down&&key==osgGA::GUIEventAdapter::KEY_A) {
        m_viewer->coordAxes().toggleVisible();
        return true;
      } else if (ctrl_down&&key==osgGA::GUIEventAdapter::KEY_Down) {
        m_viewer->coordAxes().scaleDown(true);
        std::cout<<"Viewer: New axis length: "<<G4OSG::BestUnit(m_viewer->coordAxes().getScale(), "Length")<<std::endl;//todo: visual feedback
        return true;
      } else if (ctrl_down&&key==osgGA::GUIEventAdapter::KEY_Up) {
        m_viewer->coordAxes().scaleUp(true);
        std::cout<<"Viewer: New axis length: "<<G4OSG::BestUnit(m_viewer->coordAxes().getScale(), "Length")<<std::endl;//todo: visual feedback
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_Down||key==osgGA::GUIEventAdapter::KEY_Up) {
        double factor = (key==osgGA::GUIEventAdapter::KEY_Down?-1:1)*camManip->getWheelZoomFactor();
        if (shift_down) factor*=0.2;
        else if (ctrl_down) factor*=0.02;
        camManip->zoomModel(factor,true);
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_Down||key==osgGA::GUIEventAdapter::KEY_Up) {
        double factor = (key==osgGA::GUIEventAdapter::KEY_Down?-1:1)*camManip->getWheelZoomFactor();
        if (shift_down) factor*=0.2;
        else if (ctrl_down) factor*=0.02;
        camManip->zoomModel(factor,true);
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_Left||key==osgGA::GUIEventAdapter::KEY_Right) {
        double theta=10*M_PI/180.0;//10 degrees
        if (shift_down) theta*=0.1;//1 degree
        else if (ctrl_down) theta*=0.01;//0.1 degrees
        if (key==osgGA::GUIEventAdapter::KEY_Left) theta *= -1;//opposite direction
        osg::Vec3d eye, center, up;
        camManip->getTransformation( eye, center, up );
        up.normalize();
        osg::Vec3d b(center-eye);b.normalize();
        osg::Vec3d c(b^up);c.normalize();
        camManip->setTransformation( eye, center, c*sin(theta)+up*cos(theta) );
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_F2||(key==osgGA::GUIEventAdapter::KEY_T&&ctrl_down)) {
        m_viewer->hud().toggleHUDVisibility();
        updateHoverInfo(view,ea);
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_Y) {
        m_viewer->toggleMultiSampling();
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_Escape) {
        //ESC gets out of HELP first, selection second, FullScreen this.
        if (isHelp()) {
          toggleHelp();
        } else if (vhCommon&&!vhCommon->selectedHandles().empty()) {
          vhCommon->clearAllSelectedHandles();
        } else if (m_viewer->isFullScreen()) {
          m_viewer->toggleFullScreen();
          updateHoverInfo(view,ea);
        }
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_Q&&ctrl_down) {
        m_viewer->viewer().setDone(true);
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_P) {
        camManip->toggleOrtho();
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_U) {
        m_viewer->measurementPoints().toggleVisible();
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_R&&ctrl_down) {
        m_viewer->measurementPoints().clearPoint(G4OSG::MeasurementPoints::RED);
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_G&&ctrl_down) {
        m_viewer->measurementPoints().clearPoint(G4OSG::MeasurementPoints::GREEN);
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_B&&ctrl_down) {
        m_viewer->measurementPoints().clearPoint(G4OSG::MeasurementPoints::BLUE);
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_F5||key==osgGA::GUIEventAdapter::KEY_F) {
        m_viewer->toggleFullScreen();
        updateHoverInfo(view,ea);
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_K) {
        //fixme: should also change box/text colours
        m_bgdColor = (m_bgdColor+1)%6;
        switch(m_bgdColor) {
          //NOTE: Keep the case=0 value in synch with initial value in Viewer.cc!!!!:
        case 0: cam->setClearColor(osg::Vec4(0.4,0.4,0.4,1)); break;
        case 1: cam->setClearColor(osg::Vec4(0.2,0.2,0.4,1)); break;
        case 2: cam->setClearColor(osg::Vec4(0.0,0.0,0.0,1)); break;
        case 3: cam->setClearColor(osg::Vec4(0.7,0.7,0.7,1)); break;
        case 4: cam->setClearColor(osg::Vec4(0.85,0.85,0.85,1)); break;
        case 5: cam->setClearColor(osg::Vec4(1.0,1.0,1.0,1)); break;
        default: assert(false); break;
        }

        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_T) {
        if (vhCommon) {
          if (shift_down)
            vhCommon->previousDisplayStyle();
          else
            vhCommon->nextDisplayStyle();
        }
        return true;
      } else if (key==osgGA::GUIEventAdapter::KEY_D||key==osgGA::GUIEventAdapter::KEY_I) {
        if (!vhCommon)
          return true;
        unsigned nphi = vhCommon->getNPhiRendered();
        unsigned delta = (shift_down?5:1);
        if (key==osgGA::GUIEventAdapter::KEY_D&&nphi>=4) {
          delta = (delta>nphi-3 ? nphi-3 : delta);
          nphi-=delta;
        } else if (key==osgGA::GUIEventAdapter::KEY_I&&nphi<10000) {
          nphi+=delta;
        }
        vhCommon->setNPhiRendered(nphi);
        return true;
      } else {
        m_keys_pressed.insert(key);
      }
      return false;
    case(osgGA::GUIEventAdapter::KEYUP):
      if (m_keys_pressed.count(key))
        m_keys_pressed.erase(key);
      return false;

    case(osgGA::GUIEventAdapter::DRAG):
      m_push_x = m_push_y = -1e99;
      updateHoverInfo(view,ea);
      return false;

    case(osgGA::GUIEventAdapter::PUSH):
      //record pos so we can ignore dragging
      m_push_x = ea.getX();
      m_push_y = ea.getY();
      return false;

    case(osgGA::GUIEventAdapter::RELEASE):
      {
        if (std::abs<double>(ea.getX()-m_push_x)>1.0e-5||std::abs<double>(ea.getY()-m_push_y)>1.0e-5)
          return false;
        m_push_x=m_push_y=-1e99;

        osg::Vec3d pickedPoint;
        osg::Vec3d pickedNormal;
        G4OSG::VolHandle* vh = pick(view,ea,pickedPoint,pickedNormal);
        if (!vh) {
          if (vhCommon)
            vhCommon->clearAllSelectedHandles();
          return false;
        }

        MOUSE_ACTION action(NONE);

        //NB: The keys FIXME: the keys here (like 'M','A'!!) can not be used for shortcuts as well
        if (left_click) {
          if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_S)) action = CENTER_VIEW_ON_POINT;
          else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_C)) action = CENTER_VIEW_ON_CORNER;
          else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_O)) action = ORIENT_TO_SURFACE;
          else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_M)) action = ZOOM_TO_OBJECT;
          else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_X)) action = GEO_EXPAND_TO_DAUGHTERS;
          else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_L)) action = GEO_CONTRACT_TO_MOTHER;
          else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_W)) action = GEO_WIREFRAME_EXPAND;
          else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_Z)) action = GEO_ZAP_VOLUME;
          //DISABLE else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_L)) action = GEO_SELECT_BY_LOGVOL;
          //DISABLE else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_M)) action = GEO_SELECT_BY_MATERIAL;//fixme bad key
          else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_R)) {
            action = shift_down ? MEASPOINT_MOVE_RED_TO_CORNER : MEASPOINT_MOVE_RED;
          } else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_G)) {
            action = shift_down ? MEASPOINT_MOVE_GREEN_TO_CORNER : MEASPOINT_MOVE_GREEN;
          } else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_B)) {
            action = shift_down ? MEASPOINT_MOVE_BLUE_TO_CORNER : MEASPOINT_MOVE_BLUE;
          } else if (m_keys_pressed.count(osgGA::GUIEventAdapter::KEY_A)) {
            action = shift_down ? COORD_AXES_PLACE_ON_VOL_CORNER : COORD_AXES_PLACE_ON_VOL;
          }
          else
            //DISABLE multi-select action = shift_down ? GEO_SELECT_VOL_MULTI : GEO_SELECT_VOL;
            action = GEO_SELECT_VOL;
        } else if (right_click) {
          if (ctrl_down)
            action = ORIENT_TO_SURFACE;
          else
            action = (shift_down ? CENTER_VIEW_ON_CORNER : CENTER_VIEW_ON_POINT);
        } else if (middle_click) {
          if (ctrl_down)
            action = GEO_ZAP_VOLUME;
          else
            action = shift_down ? GEO_CONTRACT_TO_MOTHER : GEO_EXPAND_TO_DAUGHTERS;
        } else {
          //should not really happen.
          printf("Viewer: WARNING - release event but no mouse clicks.");
          return false;
        }

        assert(action!=NONE);

        switch (action) {
        case CENTER_VIEW_ON_POINT:
          {
            camManip->setCenter(pickedPoint);
            return true;
          }
        case CENTER_VIEW_ON_CORNER:
          {
            osg::Vec3d corner;
            double dist_to_corner;
            bool ok = vh->findNearestCorner(pickedPoint,corner, dist_to_corner);
            if (ok) {
              camManip->setCenter(corner);
              //   if (vhCommon) vhCommon->sceneHook()->addChild(createPoint(corner.x(),corner.y(),corner.z()));
            } else {
              printf("problems finding corner\n");
            }
            return true;
          }
        case ORIENT_TO_SURFACE:
          {
            //fixme: if direction is already OK, then it is time to align "r-phi" with the edges of the surfaces!
            osg::Vec3d eye, center, up;
            camManip->getTransformation( eye, center, up );
            osg::Vec3d dir(eye-center);
            double dist = dir.length();
            //pickedPoint=center;//to not change center
            if (pickedNormal*(eye-pickedPoint)<=0.0) {
              //Due to a bug it appears that sometimes the front face is not picked,
              //but rather a face behind. To at least ignore such cases we check
              //that the normal is pointing towards the camera:
              printf("Failed to pick forward facing surface (OSG bug?). Try again.\n");
            } else {
              camManip->setTransformation( pickedPoint + pickedNormal*dist, pickedPoint, up );
              // printf("TKTEST A + PICKED physvol %s: orient to surface with normal (%g,%g,%g)\n",
              //        vh->g4PhysVol()->GetName().c_str(),
              //        pickedNormal.x(),pickedNormal.y(),pickedNormal.z());
            }
            return true;
          }
        case GEO_EXPAND_TO_DAUGHTERS:
          {
            if (vh->nDaughters()) {
              vh->hide();
              vh->displayDaughters();
              updateHoverInfo(view,ea);
            }
            return true;
          }
        case GEO_WIREFRAME_EXPAND:
          {
            if (vh->nDaughters()) {
              vh->displayDaughters();
              vh->setDisplayStyle(VolHandle::WIREFRAME);
              updateHoverInfo(view,ea);
            } else {
              //Toggle wireframe on/off for this volume (note, this does not make it inactive, so can be toggled back!)
              auto ds = vh->displayStyle();
              if (ds==VolHandle::WIREFRAME)
                vh->setDisplayStyle(VolHandle::NORMAL);
              else if (ds==VolHandle::NORMAL)
                vh->setDisplayStyle(VolHandle::WIREFRAME);
            }
            return true;
          }
        case GEO_CONTRACT_TO_MOTHER:
          {
            G4OSG::VolHandle * mother = vh->mother();
            if (mother) {
              mother->setDisplayStyle(VolHandle::NORMAL);
              mother->display();
              mother->hideDaughtersRecursively();
              updateHoverInfo(view,ea);
            }
            return true;
          }
        case GEO_ZAP_VOLUME:
          {
            vh->zap();
            updateHoverInfo(view,ea);
            return true;
          }
        case GEO_SELECT_VOL:
          {
            if (vhCommon) {
              vhCommon->toggleSelectHandle(vh,false);
              m_select_timer.setStartTick();
            }
            return true;
          }
        //DISABLE case GEO_SELECT_VOL_MULTI:
        //DISABLE   {
        //DISABLE     if (vhCommon) vhCommon->toggleSelectHandle(vh,true);
        //DISABLE     return true;
        //DISABLE   }
        case MEASPOINT_MOVE_RED_TO_CORNER:
        case MEASPOINT_MOVE_GREEN_TO_CORNER:
        case MEASPOINT_MOVE_BLUE_TO_CORNER:
        case COORD_AXES_PLACE_ON_VOL_CORNER:
          {
            osg::Vec3d corner;
            double dist_to_corner;
            bool ok = vh->findNearestCorner(pickedPoint,corner, dist_to_corner);
            if (ok) {
              if (action==COORD_AXES_PLACE_ON_VOL_CORNER) {
                m_viewer->coordAxes().setPos(corner,true);
              } else {
                if (action==MEASPOINT_MOVE_RED_TO_CORNER)
                  m_viewer->measurementPoints().setPoint(G4OSG::MeasurementPoints::RED,vh,corner,osg::Vec3d(0,0,0));
                else if (action==MEASPOINT_MOVE_GREEN_TO_CORNER)
                  m_viewer->measurementPoints().setPoint(G4OSG::MeasurementPoints::GREEN,vh,corner,osg::Vec3d(0,0,0));
                else if (action==MEASPOINT_MOVE_BLUE_TO_CORNER)
                  m_viewer->measurementPoints().setPoint(G4OSG::MeasurementPoints::BLUE,vh,corner,osg::Vec3d(0,0,0));
              }
            } else {
              printf("problems finding corner\n");
            }
            return true;
          }
        case COORD_AXES_PLACE_ON_VOL:
          {
            m_viewer->coordAxes().setPos(pickedPoint,true);
            return true;
          }
        case MEASPOINT_MOVE_RED:
          {
            //TODO,can we query the G4 geometry to get double-precision pickedPoint and pickedNormal?
            m_viewer->measurementPoints().setPoint(G4OSG::MeasurementPoints::RED,vh,pickedPoint,pickedNormal);
            return true;
          }
        case MEASPOINT_MOVE_GREEN:
          {
            m_viewer->measurementPoints().setPoint(G4OSG::MeasurementPoints::GREEN,vh,pickedPoint,pickedNormal);
            return true;
          }
        case MEASPOINT_MOVE_BLUE:
          {
            m_viewer->measurementPoints().setPoint(G4OSG::MeasurementPoints::BLUE,vh,pickedPoint,pickedNormal);
            return true;
          }
        case ZOOM_TO_OBJECT:
          //really "zoom to volume" for now.
          if (vh) {
            const osg::Node * cptr = vh->osgTrf().get();
            assert(cptr);
            camManip->zoomToNode(*const_cast<osg::Node*>(cptr));
          }
          return true;
            // case GEO_SELECT_BY_LOGVOL:
        // case GEO_SELECT_BY_MATERIAL:
        // case GEO_SELECT_BY_LOGVOL_MULTI:
        // case GEO_SELECT_BY_MATERIAL_MULTI:
        //   {
        //     printf("not yet implemented\n");
        //     return true;
        //   }
        case NONE:
          assert(false);
        }
	return false;//should never happen
      }
    default:
      return false;
    }
}

G4OSG::VolHandle* G4OSG::EventHandler::pick(osgViewer::View* view,
                                            const osgGA::GUIEventAdapter& ea,
                                            osg::Vec3d& pickedPoint,
                                            osg::Vec3d& pickedNormal)
{
  if (!view)
    return 0;
  pickedPoint.set(0,0,0);
  osgUtil::LineSegmentIntersector::Intersections intersections;

  std::string gdlist="";
  double x = ea.getX();
  double y = ea.getY();
  if (view->computeIntersections(x,y,intersections))
    {
      // osg::Vec3d startVertex, endVertex;
      // bool ok = findRay(x,y,view,startVertex,endVertex);
      // assert(ok);
       for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
          hitr != intersections.end();
          ++hitr)
        {
          pickedPoint = hitr->getWorldIntersectPoint();
          pickedNormal = hitr->getWorldIntersectNormal();

          //Find first Transform node from the back:
          auto itNodeE = hitr->nodePath.rend();
          for (auto itNode = hitr->nodePath.rbegin(); itNode!= itNodeE; ++itNode)
            {
              osg::Transform * trfnode = (*itNode)->asTransform();
              if (trfnode&&trfnode->getUserData()) {
                ObjectHolder* oh = static_cast<ObjectHolder*>(trfnode->getUserData());
                if (oh->type()==1) {
                  auto vh = oh->handle<VolHandle>();
                  if (vh->displayStyle()!=VolHandle::NORMAL && vh->nDaughters()>0)
                    continue;//we ignore wireframe/transparent if vol has daughters (it is expanded)
                  osg::Vec3d pp,pn;
                  // if (vh->rayIntersection(startVertex,endVertex-startVertex,pp,pn)) {
                  //   printf("hurra pn=(%g,%g,%g,)\n",pn.x(),pn.y(),pn.z());
                  // }
                  osg::Vec3d g4normal;
                  if (false&&vh->G4OSG::VolHandle::findNormal(pickedPoint,g4normal)) {
                    //override GL normal with normal computed by G4 geometry (might not work for curved surfaces!)
                    pickedNormal=g4normal;
                    printf("using g4 normal\n");
                  }
                  pickedNormal.normalize();
                  return vh;
                }
              }
            }
        }
    }
  return 0;
}

G4OSG::TrkHandle* G4OSG::EventHandler::pickTrack(osgViewer::View* view,
                                                 const osgGA::GUIEventAdapter& ea,
                                                 osg::Vec3d& pickedPoint,
                                                 unsigned& pickedStep)
{
  if (!view)
    return 0;
  if (!m_viewer->hasGriffTracks())
    return 0;

  pickedPoint.set(0,0,0);
  double x = ea.getX();
  double y = ea.getY();

  double w = 5.0f;
  double h = 5.0f;
  auto picker = new osgUtil::PolytopeIntersector( osgUtil::Intersector::WINDOW, x-w, y-h, x+w, y+h );
  picker->setDimensionMask( osgUtil::PolytopeIntersector::DimOne );//only look for lines
  osgUtil::IntersectionVisitor iv(picker);
#if OSG_VERSION_GREATER_OR_EQUAL(3,3,6)
  iv.getIntersector()->setPrecisionHint(osgUtil::Intersector::USE_DOUBLE_CALCULATIONS);
#endif
  iv.getIntersector()->setIntersectionLimit(osgUtil::Intersector::LIMIT_NEAREST);

  auto cam = view->getCamera();
  cam->accept(iv);
  if (!picker->containsIntersections())
    return 0;
  auto itE = picker->getIntersections().end();
  for(auto it = picker->getIntersections().begin();it!=itE;++it) {
    auto itNodeE = it->nodePath.rend();
    for (auto itNode = it->nodePath.rbegin(); itNode!= itNodeE; ++itNode) {
      if ((*itNode)->getUserData()) {
        auto oh = static_cast<ObjectHolder*>((*itNode)->getUserData());
        if (oh->type()==2) {
          pickedPoint = it->localIntersectionPoint;
          pickedStep = it->primitiveIndex;
          return oh->handle<TrkHandle>();
        }
      }
    }
  }
  return 0;
}

bool G4OSG::EventHandler::isHelp() const
{
  return m_viewer->hud().getText(G4OSG::HUD::BOX_CENTER_COLUMN1) == m_helptxt_left;
}

void G4OSG::EventHandler::initHelp()
{
  std::vector<std::string> l;

  l.push_back("LC=Left-Click, RC=Right-Click,");
  l.push_back("MC=Middle-Click, SH=SHIFT, CT=CTRL");

  l.push_back("");
  l.push_back("Basic view navigation:");
  l.push_back("  LC+DRAG       : Rotate");
  l.push_back("  MC+DRAG       : Pan");
  l.push_back("  RC+DRAG/WHEEL : Zoom");
  l.push_back("  LEFT/RIGHT    : Rotate");
  l.push_back("  UP/DOWN       : Zoom");

  l.push_back("");
  l.push_back("Advanced view navigation:");
  if (m_viewer->hasGeo()) {
    l.push_back("  RC/LC+S     : Center view on point");
    l.push_back("  RC+SH/LC+C  : Center view on corner");
    l.push_back("  RC+CT/LC+O  : Orient to surface");
    l.push_back("  LC+M        : Zoom to object");//NB: This is again geometry stuff... "zoom to object"?
  }
  l.push_back("  P           : Toggle perspective");
  l.push_back("  [1-3]       : View along x, y or z");//or x,y,z view of current selection?... and x->-x if already x
  // "  SH+[4-9]   *: Store view in user slot");//store across invocations? Like in a .history file?
  // "  [4-9]      *: Restore user view");
  l.push_back("  SPACE       : Home view");

  l.push_back("");
  l.push_back("Style options:");//NB: The first two of these are only working on Geant4 volumes...
  if (m_viewer->hasGeo()) {
    l.push_back("  I/D         : Rotation Steps");//or perhaps decrease option should use shift, for consistency?
    l.push_back("  T/SH+T      : Render style");
  }
  l.push_back("  K           : Background colour");
  l.push_back("  Y           : Anti-aliasing");//fixme: only add this here if hardware supports

  l.push_back("");
  l.push_back("Other:");
  l.push_back("  F/F5       : Fullscreen");//secretly ESC also leaves fullscreen
  l.push_back("  CT+Q       : Quit");
  l.push_back("  CT+H/F1    : Toggle help");//secretly ESC also hides help
  l.push_back("  CT+T/F2    : Toggle all text");
  //l.push_back("  CT+P      *: Screenshot");

  if (m_viewer->hasGeo()) {
    l.push_back("");
    l.push_back("Geant4 Volumes:");
    l.push_back("  LC          : Get info about volume");
    l.push_back("  MC/LC+X     : Expand vol to daughters");
    l.push_back("  MC+SH/LC+L  : Contract vol to mother");
    //l.push_back("  LC+??      *: Expand vol as transparent");
    l.push_back("  LC+W        : Expand vol as wireframe");
    l.push_back("  MC+CT/LC+Z  : Zap volume");
    l.push_back("  BKSPC/DEL   : Unzap last zapped volume");
    l.push_back("  CT+Z        : Unzap all zapped volumes");//todo: two times in succession should revert!
    //"  ??   : Save/restore geo");
    l.push_back("  CT+V        : Reset all volumes");//deselects all, hides all recursively (only handles already init of course)
  }
  //l.push_back("");
  //l.push_back("Select Geant4 volumes for info: LC");
  //DISABLE "Select Geant4 volumes (+SH for multi):");
  //DISABLE "  LC         : Select/deselect vol");
  //DISABLE "  LC+L      *: Select by logical vol");
  //DISABLE "  LC+M      *: Select by material");//fixme: M for measurement points?
  //DISABLE "  ??        *: Clear selection");//ESC would be good, but in fullscreen mode it is used
  //DISABLE "  TAB       *: Cycle shown info");
  //"  ??     *: Save setup");
  //"  ??     *: Restore setup");
  l.push_back("");
  l.push_back("Coordinate axes:");
  l.push_back("  CT+A       : Toggle axes");
  if (m_viewer->hasGeo()) {
    l.push_back("  LC+A       : Place on volume");
    l.push_back("  LC+SH+A    : Place on volume corner");
    l.push_back("  0          : Place at origin");//only makes sense if it can be moved in the first place
  }
  l.push_back("  CT+UP/DOWN : Scale axes");
  l.push_back("");
  //    "Cutaways:");
  //    "  ??        *: Hide/show cutaway");
  if (m_viewer->hasGeo()) {
    l.push_back("");
    l.push_back("Measurement points:");
    l.push_back("  LC+R       : Place red point");
    l.push_back("  LC+G       : Place green point");
    l.push_back("  LC+B       : Place blue point");
    l.push_back("  +SH        : Place on corner");
    l.push_back("  +CT        : Clear point");
    l.push_back("  U          : Hide/show all points");
    //colour editing, persistification
  }

  if (m_viewer->hasGriffTracks()) {
    l.push_back("");
    l.push_back("Simulated Tracks:");
    l.push_back("  <no interactions implemented yet>");
  }

  unsigned idiv = l.size()/2;
  for (; idiv<l.size();++idiv) {
    if (l.at(idiv).empty()) {
      break;
    }
  }
  for (unsigned i=0;i<idiv;++i) {
    if (!m_helptxt_left.empty())
      m_helptxt_left += "\n";
    m_helptxt_left += l.at(i);
  }
  for (unsigned i=idiv+1;i<l.size();++i) {
    if (!m_helptxt_right.empty())
      m_helptxt_right += "\n";
    m_helptxt_right += l.at(i);
  }
}

void G4OSG::EventHandler::toggleHelp(bool force_on)
{
  if (!force_on&&isHelp()) {
    m_viewer->hud().clearText(G4OSG::HUD::BOX_CENTER_COLUMN1);
    m_viewer->hud().clearText(G4OSG::HUD::BOX_CENTER_COLUMN2);
  } else {
    m_viewer->hud().setText(G4OSG::HUD::BOX_CENTER_COLUMN1,m_helptxt_left);
    m_viewer->hud().setText(G4OSG::HUD::BOX_CENTER_COLUMN2,m_helptxt_right);
  }
}

void G4OSG::EventHandler::updateHoverInfo(osgViewer::View* view, const osgGA::GUIEventAdapter& ea)
{
  G4OSG::HUD & hud = m_viewer->hud();
  if (!hud.getHUDVisibility())
    return;
  osg::Vec3d trkpt, volpt, dummy;
  bool allow_trackhover(!m_viewer->hasManyVisibleGriffTracks());
  if (!allow_trackhover) {
    const bool shift_down = ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_SHIFT;
    bool ctrl_down = ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL;
    if (ctrl_down||shift_down)
      allow_trackhover = true;
  }
  std::pair<TrkHandle *,unsigned> th;

  th.first = allow_trackhover ? pickTrack(view,ea,trkpt,th.second) : 0;
  G4OSG::VolHandle* vh = pick(view,ea,volpt,dummy);
  if (th.first&&vh) {
    auto camManip = dynamic_cast<G4OSG::CamManip*>(view->getCameraManipulator());
    osg::Vec3d eye, center, up;
    camManip->getTransformation( eye, center, up );
    if ((eye-trkpt).length2()<(eye-volpt).length2())
      vh = 0;
    else
      th.first = 0;
  }
  if (m_hoverVH!=vh||m_hoverTH!=th) {
    if (!vh&&!th.first) {
      clearHoverInfo();
      return;
    }
    m_hoverVH = vh;
    m_hoverTH = th;
    if (vh)
      hud.setText(HUD::BOTTOM_LEFT, vh->getHoverInfo());
    if (th.first)
      hud.setText(HUD::BOTTOM_LEFT, th.first->getHoverInfo(th.second));
  }
}
void G4OSG::EventHandler::clearHoverInfo()
{
  m_viewer->hud().clearText(HUD::BOTTOM_LEFT);
  m_hoverVH = 0;
  m_hoverTH.first = 0;
}
