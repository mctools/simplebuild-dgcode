#include "G4OSG/VolHandle.hh"
#include "G4OSG/ObjectHolder.hh"
#include "G4OSG/G4SolidConverter.hh"
#include "G4OSG/G4TransformConverter.hh"
#include "G4OSG/RayIntersection.hh"
#include "G4OSG/HUD.hh"
#include <cassert>
#include "G4VisAttributes.hh"
#include "G4Polyhedron.hh"
#include "G4VSolid.hh"
#include "G4Material.hh"
#include "WireframeUtils.hh"

#include <osg/BlendFunc>
#include <sstream>

G4OSG::VolHandle::~VolHandle(){
  assert(!m_motherHandle&&"only VolHandles for top volumes should be deleted directly");
  if (!m_daughterHandles.empty()) {
    auto itE=m_daughterHandles.end();
    for (auto it=m_daughterHandles.begin();it!=itE;++it) {
      (*it)->m_motherHandle = 0;//avoid triggering the assert above!
      delete *it;
    }
  }
}

void G4OSG::VolHandle::initDaughters() const {
  m_daughterInit = true;
  auto lv = m_physVol->GetLogicalVolume();
  unsigned n(lv->GetNoDaughters());
  m_daughterHandles.reserve(n);
  for (unsigned i=0;i<n;++i) {
    m_daughterHandles.push_back(new VolHandle(m_common,lv->GetDaughter(i),const_cast<VolHandle*>(this)));
  }
}

void G4OSG::VolHandle::setupTrf() const
{
  assert(!m_osgTrf.valid());
  m_osgTrf = G4OSG::createMatrixTransform(*(g4PhysVol()));

  VolHandle * mom = mother();
  if (mom)
    m_osgTrf->postMult(mom->osgTrf()->getMatrix());
  m_osgTrf->setUserData(new ObjectHolder((void*)this,1));
  m_osgTrf->addChild(osgGeode().get());
  assert(m_osgTrf.valid());
}

void G4OSG::VolHandle::setupGeode() const
{
  //FIXME: Should share whenever the solid ptr is the same!! (or not... depending on whether we make individual colours)
  assert(!m_osgGeode.valid());
  m_osgGeode = G4OSG::g4solid2Geode(g4Solid());
  //m_osgGeode->getOrCreateStateSet()->setMode(GL_MULTISAMPLE_ARB, osg::StateAttribute::ON);

  //FIXME: Test colours:
  auto va = g4LogVol()->GetVisAttributes();

  auto g4col = va ? va->GetColour() : G4Colour(0.5,0.5,0.5);
  osg::Vec4Array* colors = new osg::Vec4Array;
  colors->push_back(osg::Vec4(g4col.GetRed(), g4col.GetGreen(),g4col.GetBlue(),g4col.GetAlpha()) ); //index 0 red
  auto * geo = dynamic_cast<osg::Geometry*>(m_osgGeode->getDrawable( 0 ));
  assert(geo);
  geo->setColorArray(colors);
  geo->setColorBinding(osg::Geometry::BIND_OVERALL);

  osg::StateSet* set = geo->getOrCreateStateSet();
  set->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
  set->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA ,GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);
  set->setMode(GL_BLEND, osg::StateAttribute::ON);

  if (m_displayStyle==WIREFRAME)
    G4OSG::forcedWireFrameModeOn( m_osgGeode.get() );
  assert(m_displayStyle!=TRANSPARENT);//unsupported

  assert(m_osgGeode.valid());
}

void G4OSG::VolHandle::display()
{
  if (m_displayed)
    return;
  assert(!isRendered());
  m_displayed=true;
  if (isRendered()) {
    if (isHighlighted()) {
      m_common->sceneHookHighlighted()->addChild(osgTrf());
    } else {
      m_common->sceneHookNormal()->addChild(osgTrf());
    }
  }
}

void G4OSG::VolHandle::unzap()
{
  if (!m_zapped)
    return;
  assert(!isRendered());
  m_zapped=false;
  if (isRendered()) {
    if (isHighlighted()) {
      m_common->sceneHookHighlighted()->addChild(osgTrf());
    } else {
      m_common->sceneHookNormal()->addChild(osgTrf());
    }
  }
  m_common->registerUnzap(this);
}

void G4OSG::VolHandle::hide()
{
  if (!m_displayed)
    return;

  if (isRendered()) {
    assert(m_osgTrf.valid());
    if (isHighlighted())
      m_common->sceneHookHighlighted()->removeChild(osgTrf());
    else
      m_common->sceneHookNormal()->removeChild(osgTrf());
  }
  m_displayed=false;
  assert(!isRendered());

  //hiding a volume deselects it: (fixme: only if selected!!)
  m_common->deselectHandle(this,true);
}

void G4OSG::VolHandle::zap()
{
  if (m_zapped)
    return;

  if (isRendered()) {
    assert(m_osgTrf.valid());
    if (isHighlighted())
      m_common->sceneHookHighlighted()->removeChild(osgTrf());
    else
      m_common->sceneHookNormal()->removeChild(osgTrf());
  }
  m_zapped=true;
  assert(!isRendered());
  m_common->registerZap(this);
}

void G4OSG::VolHandle::setDisplayStyle(DISPLAYSTYLE ds) {
  if (m_displayStyle==ds)
    return;
  m_displayStyle=ds;
  assert(ds!=TRANSPARENT&&"TRANSPARENCY NOT IMPLEMENTED YET");
  if (m_displayStyle==WIREFRAME)
    G4OSG::forcedWireFrameModeOn( m_osgGeode.get() );
  else
    G4OSG::forcedWireFrameModeOff( m_osgGeode.get() );
}

void G4OSG::VolHandle::setDisplayStyleRecursively(DISPLAYSTYLE ds) {
  if (m_displayStyle!=ds)
    return;
  setDisplayStyle(ds);
  if (!m_daughterInit) {
    if (ds==NORMAL)
      return;//no need to dive further, starts out NORMAL by default
    initDaughters();
  }
  auto itE=daughtersEnd();
  for (auto it=daughtersBegin();it!=itE;++it) {
    (*it)->setDisplayStyleRecursively(ds);
  }
}


void G4OSG::VolHandle::setHighlighted(bool hl)
{
  if (m_highlighted == hl)
    return;

  m_highlighted = hl;

  if (!isRendered())
    return;

  if (hl) {
    //rendered volume going from not highlighted to highlighted
    m_common->sceneHookNormal()->removeChild(osgTrf());
    m_common->sceneHookHighlighted()->addChild(osgTrf());
  } else {
    //rendered volume going from highlighted to not highlighted
    m_common->sceneHookHighlighted()->removeChild(osgTrf());
    m_common->sceneHookNormal()->addChild(osgTrf());
  }
}

void G4OSG::VolHandle::displayDaughters()
{
  auto itE=daughtersEnd();
  for (auto it=daughtersBegin();it!=itE;++it) {
    if (!(*it)->g4MarkedInvisible())
      (*it)->display();
  }
}

void G4OSG::VolHandle::hideDaughters()
{
  if (!m_daughterInit)
    return;//no need, starts out hidden by default
  auto itE=daughtersEnd();
  for (auto it=daughtersBegin();it!=itE;++it)
    (*it)->hide();
}

void G4OSG::VolHandle::hideDaughtersRecursively()
{
  if (!m_daughterInit)
    return;//no need, starts out hidden by default
  auto itE=daughtersEnd();
  for (auto it=daughtersBegin();it!=itE;++it) {
    (*it)->hide();
    (*it)->hideDaughtersRecursively();
  }
}


bool G4OSG::VolHandle::findNormal(const osg::Vec3d& tmppoint,osg::Vec3d& normal, bool local_coords)
{
  osg::Vec3d point;
  if (local_coords)
    point = tmppoint;
  else
    point = osg::Matrixd::inverse(getMatrix()).preMult(tmppoint);//cache inverse matrix??

  /////////////////////////////////////////////////////////////////

  G4ThreeVector p(point.x(),point.y(),point.z());
  G4ThreeVector g4normal = g4Solid()->SurfaceNormal(p);//would be good to return false if p not on surface!!
  normal.set(g4normal.x(),g4normal.y(),g4normal.z());
  //Todo: need to then call DistanceToIn/out ??

  /////////////////////////////////////////////////////////////////
  if (!local_coords)
    normal = getMatrix().preMult(normal);

  return true;
}

bool G4OSG::VolHandle::getExtent( double&xmin, double&xmax,
                                  double&ymin, double&ymax,
                                  double&zmin, double&zmax)
{
  G4Polyhedron* ph = g4Solid()->GetPolyhedron();
  if (!ph)
    return false;
  const int nv(ph->GetNoVertices());
  if (nv<1)
    return false;
  auto trf = osg::Matrixd::inverse(getMatrix());
  G4Point3D v = ph->GetVertex(1);
  auto p = trf.preMult(osg::Vec3d(v.x(),v.y(),v.z()));
  xmin = xmax = p.x();
  ymin = ymax = p.y();
  zmin = zmax = p.z();
  for (int iv = 1; iv<nv; ++iv) {
    v = ph->GetVertex(iv+1);
    p = trf.preMult(osg::Vec3d(v.x(),v.y(),v.z()));
    xmin = std::min<double>(xmin,p.x());
    ymin = std::min<double>(ymin,p.y());
    zmin = std::min<double>(zmin,p.z());
    xmax = std::max<double>(xmax,p.x());
    ymax = std::max<double>(ymax,p.y());
    zmax = std::max<double>(zmax,p.z());
  }
  return true;
}

bool G4OSG::VolHandle::getExtentDaughters( double&xmin, double&xmax,
                                           double&ymin, double&ymax,
                                           double&zmin, double&zmax)
{
  auto it=daughtersBegin();
  auto itE=daughtersEnd();
  if (it==itE)
    return false;//no daughters
  if (!(*it)->getExtent(xmin,xmax,ymin,ymax,zmin,zmax))
    return false;
  double x1,x2,y1,y2,z1,z2;
  for (++it;it!=itE;++it) {
    if (!(*it)->getExtent(x1,x2,y1,y2,z1,z2))
      return false;
    xmin = std::min<double>(xmin,x1);
    ymin = std::min<double>(ymin,y1);
    zmin = std::min<double>(zmin,z1);
    xmax = std::max<double>(xmax,x2);
    ymax = std::max<double>(ymax,y2);
    zmax = std::max<double>(zmax,z2);
  }
  return true;
}

bool G4OSG::VolHandle::findNearestCorner(const osg::Vec3d& tmppoint,osg::Vec3d& corner, double& dist_to_corner, bool local_coords)
{
  osg::Vec3d point;
  if (local_coords)
    point = tmppoint;
  else
    point = osg::Matrixd::inverse(getMatrix()).preMult(tmppoint);//cache inverse matrix??

  //fixme: to helper class rather than volhandle (G4Utils: find nearest G4Point3D of polyhedron)?

  corner.set(0,0,0);
  dist_to_corner = -1;

  G4Polyhedron* ph = g4Solid()->GetPolyhedron();
  if (!ph)
    return false;

  G4Point3D p(point.x(),point.y(),point.z());
  G4Point3D p_closest;

  const int nv(ph->GetNoVertices());
  if (nv<1)
    return false;

  for (int iv = 0; iv<nv; ++iv) {
    G4Point3D v = ph->GetVertex(iv+1);
    double dist_squared = p.distance2(v);
    if (iv==0||dist_squared<dist_to_corner) {
      dist_to_corner = dist_squared;
      p_closest = v;
    }
  }

  corner.set(p_closest.x(),p_closest.y(),p_closest.z());
  dist_to_corner = sqrt(dist_to_corner);

  if (!local_coords)
    corner = getMatrix().preMult(corner);

  return true;
}

//fixme to G4Utils:
#include "G4BooleanSolid.hh"
//#include "G4BREPSolid.hh"
#include "G4CSGSolid.hh"
#include "G4GenericTrap.hh"
#include "G4Tet.hh"
#include "G4Para.hh"
#include "G4Trd.hh"
#include "G4Polyhedra.hh"
#include "G4Box.hh"
#include "G4Trap.hh"

//G4GenericTrap

//fixme: to g4utils
namespace G4Utils {
bool shapeMightBeRotational(const G4VSolid * s)
{
  if (dynamic_cast<const G4Box*>(s)) return false;
  if (dynamic_cast<const G4Para*>(s)) return false;
  if (dynamic_cast<const G4Trd*>(s)) return false;
  if (dynamic_cast<const G4Trap*>(s)) return false;
  if (dynamic_cast<const G4Polyhedra*>(s)) return false;
  if (dynamic_cast<const G4Tet*>(s)) return false;
  //fixme: boolean volumes are rotational if any parts inside are.
  const G4BooleanSolid* sb = dynamic_cast<const G4BooleanSolid*>(s);
  if (sb) {
    if (shapeMightBeRotational(sb->GetConstituentSolid(0)))
      return true;
    return shapeMightBeRotational(sb->GetConstituentSolid(1));
  }
  return true;
}
}



bool G4OSG::VolHandle::hasVisibleDaughtersRecursively()
{
  if (!m_daughterInit)
    return false;//no need to check, daughters start out as hidden

  auto itE = daughtersEnd();
  for (auto it = daughtersBegin();it!=itE;++it) {
    if ((*it)->isDisplayed())
      return true;
  }

  itE = daughtersEnd();
  for (auto it = daughtersBegin();it!=itE;++it) {
    if ((*it)->hasVisibleDaughtersRecursively())
      return true;
  }
  return false;
}

void G4OSG::VolHandle::updateRotationalShapesRecursively()
{
  //update self:
  if (m_osgGeode.valid()&&G4Utils::shapeMightBeRotational(g4Solid())) {
    bool displayed = isDisplayed();
    if (displayed)
      hide();

    //release old shape and rebuild:
    m_osgTrf->removeChild(m_osgGeode.get());
    m_osgGeode = 0;
    m_osgTrf->addChild(osgGeode());//in principle we could wait with this if hidden (if careful)

    if (displayed)
      display();
  }

  //pass on to daughters:

  if (!m_daughterInit)
    return;//no need

  auto itE = daughtersEnd();
  for (auto it = daughtersBegin();it!=itE;++it)
    (*it)->updateRotationalShapesRecursively();

}

bool G4OSG::VolHandle::rayIntersection(const osg::Vec3d&tmp_p,
                                       const osg::Vec3d&tmp_d,
                                       osg::Vec3d& pickedPoint,
                                       osg::Vec3d& pickedNormal)
{
  //to local:
  osg::Vec3d p = osg::Matrixd::inverse(getMatrix()).preMult(tmp_p);
  osg::Vec3d d = osg::Matrixd::inverse(getMatrix()).preMult(tmp_d);


  G4Polyhedron* ph = g4Solid()->GetPolyhedron();
  if (!ph)
    return false;
  G4ThreeVector g4p(p.x(),p.y(),p.z());
  G4ThreeVector g4d(d.x(),d.y(),d.z());
  G4ThreeVector g4pp;
  G4ThreeVector g4pn;
  // printf("intersect test %s\n",g4PhysVol()->GetName().c_str());
  // printf("ray (%g,%g,%g)+t(%g,%g,%g)\n",
  //        p.x(),p.y(),p.z(),
  //        d.x(),d.y(),d.z());
  if (!rayIntersectsPolyhedron(*ph,g4p,g4d,g4pp,g4pn))
    return false;
  pickedPoint.set(g4pp.x(),g4pp.y(),g4pp.z());
  pickedNormal.set(g4pn.x(),g4pn.y(),g4pn.z());

  //to global:
  osg::Vec3d tmpp(pickedPoint),tmpn(pickedNormal);
  pickedPoint = getMatrix().preMult(tmpp);
  pickedNormal = getMatrix().preMult(tmpn)-getMatrix().preMult(osg::Vec3d(0,0,0));

  return true;
}

void G4OSG::VolHandle::nameML(const std::string& in, std::string& out, unsigned l) const
{
  assert(l>4);
  out = in;
  if (out.size()>l) {
    out.resize(l-3);
    out+="...";
  }
}

std::string G4OSG::VolHandle::pvNameMaxLength(unsigned l) const
{
  std::string name;
  nameML(m_physVol->GetName(),name,l);
  return name;
}
std::string G4OSG::VolHandle::lvNameMaxLength(unsigned l) const
{
  std::string name;
  nameML(g4LogVol()->GetName(),name,l);
  return name;
}
std::string G4OSG::VolHandle::solidNameMaxLength(unsigned l) const
{
  std::string name;
  nameML(g4Solid()->GetName(),name,l);
  return name;
}

std::string G4OSG::VolHandle::matNameMaxLength(unsigned l) const
{
  std::string name;
  nameML(g4LogVol()->GetMaterial()->GetName(),name,l);
  return name;
}

std::string G4OSG::VolHandle::getHoverInfo() const
{
  std::stringstream ss;
  std::string name = pvNameMaxLength(35);
  std::string matname = matNameMaxLength(50-name.length());
  ss<<"Volume \""<<name << "\" [CopyNo "<<m_physVol->GetCopyNo()<<", mat \""<<matname<<"\"";
  if (nDaughters())
    ss<<", "<<nDaughters()<<" daughter"<<(nDaughters()==1?"":"s");
  ss<<"]" <<"\n";
  return ss.str();
}
