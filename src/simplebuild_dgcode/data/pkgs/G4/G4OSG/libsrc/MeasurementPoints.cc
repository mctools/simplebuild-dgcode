#include "G4OSG/MeasurementPoints.hh"
#include "G4OSG/Viewer.hh"
#include "G4OSG/VolHandle.hh"
#include <osg/Geode>
#include <osg/Shape>
#include <osg/AutoTransform>
#include "CLHEP/Units/SystemOfUnits.h"
#include "G4OSG/BestUnit.hh"

G4OSG::MeasurementPoints::MeasurementPoints(G4OSG::Viewer*v,osg::Group* sceneHook)
  : m_isAttached(false),
    m_sceneHookExternal(sceneHook),
    m_sceneHook(new osg::Group),
    m_viewer(v)
{
}

unsigned G4OSG::MeasurementPoints::nPointsDefined()
{
  unsigned n(0);
  for (int p=0;p<(int)NPOINTS;++p)
    if (m_points[(POINT)p].defined)
      ++n;
  return n;
}

void G4OSG::MeasurementPoints::toggleVisible()
{
  if (!nPointsDefined())
    return;
  m_viewer->safeStopThreading();
  if (m_isAttached)
    detach();
  else
    attach();
  updateInfo();
  m_viewer->safeStartThreading();
}

void G4OSG::MeasurementPoints::clearPointInternal(POINT p)
{
  //internal use - no threading changes and no info printed
  m_points[p].defined=false;
  m_sceneHook->removeChild(m_points[p].node);

  if (m_isAttached&&nPointsDefined()==0)
    detach();
}

void G4OSG::MeasurementPoints::clearPoint(POINT p)
{
  if (!m_points[p].defined)
    return;

  m_viewer->safeStopThreading();
  clearPointInternal(p);
  updateInfo();
  m_viewer->safeStartThreading();
}

void G4OSG::MeasurementPoints::setPoint(POINT pidx,
                                        VolHandle * vh,
                                        const osg::Vec3d& point,
                                        const osg::Vec3d& normal)
{
  m_viewer->safeStopThreading();

  //to avoid some caching issues we always recreate the entire geode

  clearPointInternal(pidx);

  //clear other points on the same location on the same volume:
  for (int i=0;i<(int)NPOINTS;++i) {
    if (i!=(int)pidx &&
        m_points[i].defined &&
        m_points[i].volHandle==vh &&
        (m_points[i].point-point).length()<1.0e-5)
      {
        clearPointInternal((POINT)i);
      }
  }

  Point& p = m_points[pidx];
  p.node = 0;
  p.point = point;
  p.normal = normal;
  p.volHandle = vh;

  auto geode = new osg::Geode;
  p.shapedrawable = new osg::ShapeDrawable;//fixme: mem leak?
  //fixme: enable transparency!!
  if (pidx==RED) p.shapedrawable->setColor(        osg::Vec4(1.0, 0.0, 0.0, 0.5) );
  else if (pidx==GREEN) p.shapedrawable->setColor( osg::Vec4(0.0, 1.0, 0.0, 0.5) );//perhaps animated transparency?
  else if (pidx==BLUE) p.shapedrawable->setColor(  osg::Vec4(0.0, 0.0, 1.0, 0.5) );
  else { assert(false); }

  const double shape_size_cube = 8.0;//edge of cube (fixme, this probably gives different result for very large/small model?)
  const double shape_size_sphere(shape_size_cube/1.611992);//radius giving similar volume

  if (p.isOnCorner()) {
    //corner
    //    printf("setting box at (%g,%g,%g)\n",point.x(),point.y(),point.z());
    //p.shapedrawable->setShape( new osg::Box(point,2*shape_size) );
    p.shapedrawable->setShape( new osg::Box(osg::Vec3d(0,0,0),shape_size_cube) );
  } else {
    //printf("setting sphere at (%g,%g,%g)\n",point.x(),point.y(),point.z());
    //p.shapedrawable->setShape( new osg::Sphere(point,,shape_size) );
    p.shapedrawable->setShape( new osg::Sphere(osg::Vec3d(0,0,0),shape_size_sphere) );
  }
  geode->addDrawable( p.shapedrawable.get() );
  geode->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);
  if (false) {
    geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
    geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
  }
  //  geode->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA ,GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);

  osg::AutoTransform* at = new osg::AutoTransform;
  at->addChild(geode);
  at->setAutoRotateMode(osg::AutoTransform::NO_ROTATION );
  //  at->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
  at->setAutoScaleToScreen(true);

  // at->setMinimumScale(1.0);
  // at->setMaximumScale(1.0);
  at->setPosition(point);

  p.node = at;

  if (!p.defined) {
    //attach point!
    p.defined=true;
    m_sceneHook->addChild(p.node);
  }

  if (!m_isAttached)
    attach();

  updateInfo();

  m_viewer->safeStartThreading();
}

void G4OSG::MeasurementPoints::updateInfo()
{
  printf("##################################################\n");
  printf("########### Current measurement points ###########\n");
  printf("###\n");
  unsigned n = nPointsDefined();
  if (n==0) {
    printf("### No points defined\n");
    printf("###\n");
    printf("##################################################\n");
    return;
  }
  if (!m_isAttached) {
    printf("### %i points defined but hidden\n",n);
    printf("###\n");
    printf("##################################################\n");
    return;
  }
  std::string strs[3];
  const char * colname[] = { "RED","GREEN", "BLUE" };
  for (int p=0;p<(int)NPOINTS;++p) {
    if (m_points[(POINT)p].defined) {
      auto pos = m_points[(POINT)p].point;
      strs[0] = G4OSG::BestUnit(pos.x(), "Length");
      strs[1] = G4OSG::BestUnit(pos.y(), "Length");
      strs[2] = G4OSG::BestUnit(pos.z(), "Length");
      printf("### %s point: (%s,%s,%s) on \"%s\" (copyNbr %i) %s\n",
             colname[p],
             strs[0].c_str(),strs[1].c_str(),strs[2].c_str(),
             m_points[(POINT)p].volHandle->g4PhysVol()->GetName().c_str(),
             m_points[(POINT)p].volHandle->g4PhysVol()->GetCopyNo(),
             (m_points[(POINT)p].isOnCorner()?" [on corner]":" [on surface]"));
    }
  }
  if (n>=2) {
    for (int p1=0;p1<(int)NPOINTS;++p1) {
      const Point& pt1 = m_points[(POINT)p1];
      if (!pt1.defined)
        continue;
      for (int p2=p1+1;p2<(int)NPOINTS;++p2) {
        const Point& pt2 = m_points[(POINT)p2];
        if (!pt2.defined)
          continue;
        auto v1=pt1.point;
        auto v2=pt2.point;
        auto col1 = colname[p1];
        auto col2 = colname[p2];
        auto v = v2-v1;
        strs[0] = G4OSG::BestUnit(v.length(), "Length");
        printf("### %s-%s distance: %s\n",col1,col2,strs[0].c_str());
        strs[0] = G4OSG::BestUnit(v.x(), "Length");
        strs[1] = G4OSG::BestUnit(v.y(), "Length");
        strs[2] = G4OSG::BestUnit(v.z(), "Length");
        printf("### %s-%s vector: (%s,%s,%s)\n",col1,col2,
               strs[0].c_str(),strs[1].c_str(),strs[2].c_str());
        strs[0] = G4OSG::BestUnit(0.5*(v1.x()+v2.x()), "Length");
        strs[1] = G4OSG::BestUnit(0.5*(v1.y()+v2.y()), "Length");
        strs[2] = G4OSG::BestUnit(0.5*(v1.z()+v2.z()), "Length");
        printf("### %s-%s midpoint: (%s,%s,%s)\n",col1,col2,
               strs[0].c_str(),strs[1].c_str(),strs[2].c_str());
        //If on parallel surfaces which are likely to be different facets
        //(i.e. on different volumes or have a non-zero distance), we print out
        //the surface-to-surface distance:
        if (!pt1.isOnCorner()&&!pt2.isOnCorner()) {
          //both points are on a surface, so we can possibly provide angular
          //or distance info:
          double dotp = pt1.normal * pt2.normal;
          double costh = dotp / (pt1.normal.length()*pt2.normal.length());
          if (std::abs<double>(costh) < 1.0-1.0e-5) {
            //non-parallel surfaces
            printf("### %s-%s surface angle: %.10g deg\n",col1,col2,acos(std::abs<double>(costh))*180/M_PI);
          } else {
            //parallel surfaces
            printf("### %s-%s surfaces are parallel!\n",col1,col2);
            //To find the distance, we first figure out where the line given by
            //p1+n1*d intersects the (p2,n2) plane:
            double d = (v2-v1) * pt2.normal/ dotp;
            //The distance is then:
            double dist = std::abs<double>(d * pt1.normal.length());
            if (pt1.volHandle!=pt2.volHandle||dist>1.0e-3) {
              std::string diststr = G4OSG::BestUnit(dist, "Length");
              printf("### %s-%s dist between surface planes: %s\n",col1,col2,diststr.c_str());
            }
          }


        }
        //fixme: if normal is set, also display distance to surface!!
      }
    }
  }
  if (n==3) {
    printf("### angular info here...\n");
  }
  printf("###\n");
  printf("##################################################\n");
}
