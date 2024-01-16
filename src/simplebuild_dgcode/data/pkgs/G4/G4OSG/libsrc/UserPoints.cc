#include "G4OSG/UserPoints.hh"
#include "G4OSG/Viewer.hh"
#include <osg/Geode>
#include <osg/Shape>
#include <osg/Point>
#include <osg/BlendFunc>
#include <sstream>
#include <iostream>
#include "Utils/ReadAsciiNumbers.hh"

G4OSG::UserPoints::UserPoints(const std::string& datafile,
                              G4OSG::Viewer*v,
                              osg::Group* sceneHook)
  : m_isAttached(false),
    m_sceneHookExternal(sceneHook),
    m_sceneHook(new osg::Group),
    m_viewer(v)
{
  std::vector<double> tmp;
  if (!Utils::read_numbers(datafile, tmp)) {
    std::stringstream ss_errmsg;
    ss_errmsg << "Problems reading coordinates from file "<<datafile<<" (after "<<tmp.size()<<" numbers read)";
    throw std::runtime_error(ss_errmsg.str().c_str());
  }

  if (tmp.size()%3) {
    std::stringstream ss_errmsg;
    ss_errmsg << "Problems reading coordinates from file "<<datafile<<" (found "<<tmp.size()<<" numbers which is not a multiple of 3)";
    throw std::runtime_error(ss_errmsg.str().c_str());
  }
  if (tmp.empty()) {
    std::stringstream ss_errmsg;
    ss_errmsg << "Did not find any coordinates in file "<<datafile;
    throw std::runtime_error(ss_errmsg.str().c_str());
  }
  m_points.reserve(tmp.size()/3);
  for(auto it = tmp.begin();it!=tmp.end();) {
    double x = *it++;
    double y = *it++;
    double z = *it++;
    m_points.emplace_back(x,y,z);
  }
  std::cout<<"Viewer: Loaded "<<m_points.size()<<" points from "<<datafile<<std::endl;
  createPoints();
}

osg::Geode * G4OSG::UserPoints::createPoints(double ptsize, double r, double g, double b, double alpha)
{
  auto geode = new osg::Geode;

  osg::Geometry* geo = new osg::Geometry;
  geode->addDrawable( geo );

  //NB: would like to use Vec3dArray, but it gives a warning from TriangleFunctor...
  auto points = new osg::Vec3Array;
  points->reserve(m_points.size());
  for( auto it = m_points.begin(); it!=m_points.end(); ++it)
    points->push_back( *it );

  geo->setVertexArray( points );
  auto draw = new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, m_points.size());

  geo->addPrimitiveSet( draw );
  osg::Vec4Array* colors = new osg::Vec4Array;
  colors->push_back(osg::Vec4(r,g,b,alpha));
  geo->setColorArray(colors);
  geo->setColorBinding(osg::Geometry::BIND_OVERALL);
  osg::StateSet* state = geode->getOrCreateStateSet();//leak ?
  state->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
  state->setMode( GL_BLEND, osg::StateAttribute::ON );
  if (alpha!=1.0)
    state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
  state->setMode( GL_POINT_SMOOTH, osg::StateAttribute::ON );

  if (ptsize!=1.0)
    state->setAttribute( new osg::Point( (float)ptsize ), osg::StateAttribute::ON );

  //If we had not disabled SMALL_FEATURE_CULLING on our camera, we would have to
  //enable the following code to show point sets with zero extent:
  //osg::BoundingSphere bs(m_points.front(),1.0);;
  //geode->setInitialBound(bs);

  return geode;

}

void G4OSG::UserPoints::createPoints()
{
  assert(!m_points.empty());
  if (m_points.size()<1000) {
    m_sceneHook->addChild( createPoints(8.0,0.8,0,0,0.5) );
    m_sceneHook->addChild( createPoints(3.0,0.8,0.6,0.6,0.7) );
    m_sceneHook->addChild( createPoints(1.0,1,1,1,1.0) );
  } else {
    m_sceneHook->addChild( createPoints(3.0,1,0,0,1.0) );
  }
}

void G4OSG::UserPoints::toggleVisible()
{
  m_viewer->safeStopThreading();
  if (m_isAttached)
    detach();
  else
    attach();
  m_viewer->safeStartThreading();
}
