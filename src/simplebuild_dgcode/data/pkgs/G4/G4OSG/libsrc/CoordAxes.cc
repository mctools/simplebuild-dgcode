#include "G4OSG/CoordAxes.hh"
#include "G4OSG/Viewer.hh"
#include "G4SystemOfUnits.hh"

#include "Core/FindData.hh"//NB: Is this our only dependency on other packages?
#include "Core/String.hh"//NB: Is this our only dependency on other packages?
#include <osgDB/ReadFile>
#include <string>
#include <stdexcept>
#include <cstdlib>

G4OSG::CoordAxes::CoordAxes(Viewer*viewer,osg::Group* sceneHook)
  : m_viewer(viewer),
    m_sceneHook(sceneHook),
    m_trf(new osg::MatrixTransform),
    m_pos(0,0,0),
    m_scalemod(0),
    m_scale_base_nanometer(1000000000),
    m_visible(false)
{
  assert(viewer);
  assert(sceneHook);

  //
  std::string origin_override(Viewer::envStr("G4OSG_COORDAXISORIGIN"));
  if (!origin_override.empty()) {
    std::vector<std::string> parts;
    Core::split(parts,origin_override,",;");
    if (parts.size()!=3)
      throw std::runtime_error("Invalid length of vector in G4OSG_COORDAXISORIGIN env var");
    for(unsigned i = 0; i <parts.size();++i)
      m_pos[i] = std::stod(parts.at(i));
  }

  // Support alternative axes, such as the one from:
  // https://github.com/openscenegraph/OpenSceneGraph-Data/raw/master/axes.osgt

  auto alternative_axesmodelfile = std::getenv("DGCODE_G4OSG_ALTERNATIVEAXESFILE");

  osg::ref_ptr<osg::Node> model_axes;
  if ( alternative_axesmodelfile ) {
    std::cout << "Viewer: Trying to load alternative axes model from file: "
              << alternative_axesmodelfile << std::endl;
    model_axes = osgDB::readNodeFile( alternative_axesmodelfile );
  } else {
    model_axes = createAxesModel();
  }

  //Make sure normals are normalised when axes are scaled:
  model_axes->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);

  m_trf->addChild( model_axes.get() );

  updateTrf();

}

G4OSG::CoordAxes::~CoordAxes()
{
}

void G4OSG::CoordAxes::setPos(const osg::Vec3d&pos,bool makeVisible)
{
  m_viewer->safeStopThreading();
  if (m_pos!=pos) {
    m_pos=pos;
    updateTrf();
  }
  //make visible when pos is changed (makes sense from UI POV):
  if (makeVisible&&!m_visible) {
    m_visible=true;
    m_sceneHook->addChild( m_trf.get() );
  }
  m_viewer->safeStartThreading();
}

double G4OSG::CoordAxes::getScale() const
{
  double fact(m_scalemod?(m_scalemod==1?0.2:0.5):1.0);
  return (fact * CLHEP::nanometer) * m_scale_base_nanometer;
}

void G4OSG::CoordAxes::scaleUp(bool makeVisible)
{
  //We make sure we scale up and down between nice numbers (.. -> 1 cm -> 2cm -> 5cm -> 10cm -> 20cm -> 50cm -> 1meter -> ...)
  if (m_scalemod == 0 && m_scale_base_nanometer >= 1000000000000000000)
    return;//max 1 gigameter
  m_scalemod = (m_scalemod+1)%3;
  if (m_scalemod==1)
    m_scale_base_nanometer *= 10;
  scaleChanged(makeVisible);
}

void G4OSG::CoordAxes::scaleDown(bool makeVisible)
{
  if (m_scalemod == 0 && m_scale_base_nanometer == 1)
    return;//min 1 nanometer
  m_scalemod = (m_scalemod-1+3)%3;
  if (m_scalemod==0)
    m_scale_base_nanometer /= 10;
  scaleChanged(makeVisible);
}

void G4OSG::CoordAxes::overrideScale(double val,bool makeVisible)
{
  m_scale_base_nanometer = val*1e6;//mm to nanometer
  m_scalemod = 0;
  scaleChanged(makeVisible);
}


void G4OSG::CoordAxes::scaleChanged(bool makeVisible)
{
  bool vis(makeVisible||m_visible);
  if (vis)
    m_viewer->safeStopThreading();
  updateTrf();
  if (makeVisible&&!m_visible) {
    m_visible=true;
    m_sceneHook->addChild( m_trf.get() );
  }
  if (vis)
    m_viewer->safeStartThreading();
}

void G4OSG::CoordAxes::toggleVisible()
{
  m_viewer->safeStopThreading();
  m_visible=!m_visible;
  if (m_visible) {
    m_sceneHook->addChild( m_trf.get() );
  } else {
    m_sceneHook->removeChild( m_trf.get() );
  }
  m_viewer->safeStartThreading();
}

void G4OSG::CoordAxes::updateTrf()
{
  //The tip of the arrows in the model axes.osgt actually extend to 1.2, so we
  //scale it down accordingly:
  double scale = getScale();
  m_trf->setMatrix( osg::Matrix::scale(scale,scale,scale)*
                    osg::Matrix::translate(m_pos));
}

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osgDB/ReadFile>
#include <osg/Quat>
//#include <osgDB/WriteFile>

osg::ref_ptr<osg::Node> G4OSG::CoordAxes::createAxesModel()
{
  osg::ref_ptr<osg::Group> axes_group = new osg::Group;

  //Our own custom alternative to open-scene-graph's axes.osgt which sadly was
  //not released under a compatible open source license. Our own implementation
  //here is just a few cylinders + cones, and Billboard'ed letters loaded from
  //files kindly created and donated to dgcode by Peter Willendrup in STL
  //format, after which osgconv was subsequently used to bring them into OSG
  //format + used to rescale/translate a bit. We also edited the files to change
  //the Geode to Billboard:

  //Free parameters of dimensions of cylinder and cone composing the arrows:
  constexpr double cyl_height = 0.8;
  constexpr double cyl_radius = 0.018;
  constexpr double cone_radius = 2.5*cyl_radius;

  //Derived parameters:
  constexpr double cone_height = 1.0 - cyl_height;
  constexpr double cone_intended_offset = 1.0 - 0.5 * cone_height;
  constexpr double cone_baseoffsetfactor = 0.25f;//Somewhat surprising hardcoded offset in osg::Cone
  constexpr double cone_offset = cone_intended_offset-cone_baseoffsetfactor*cone_height;
  constexpr double cyl_offset = 0.5* cyl_height;
  constexpr double halfpi = 1.5707963267948966192313216916397514420985847;

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////// X ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  {
    auto colors_red = new osg::Vec4Array;
    colors_red->push_back( osg::Vec4(1,0,0,1) );

    auto cyl_center = osg::Vec3( cyl_offset, 0.0, 0.0 );
    auto cone_center = osg::Vec3( cone_offset, 0.0, 0.0 );

    auto cylinder = new osg::Cylinder(cyl_center, cyl_radius, cyl_height);
    osg::Quat rot_z_to_x;
    rot_z_to_x.makeRotate(  halfpi, 0.0, 1.0, 0.0 );
    cylinder->setRotation(rot_z_to_x);

    osg::ref_ptr<osg::ShapeDrawable> cylDrawable = new osg::ShapeDrawable(cylinder);
    cylDrawable->setColorArray(colors_red);
    cylDrawable->setColorBinding(osg::Geometry::BIND_OVERALL);
    auto cone = new osg::Cone(cone_center, cone_radius, cone_height);
    if ( cone->getBaseOffsetFactor() != cone_baseoffsetfactor )
      throw std::runtime_error("Unexpected osg::Cone BaseOffsetFactor");

    cone->setRotation(rot_z_to_x);

    osg::ref_ptr<osg::ShapeDrawable> coneDrawable = new osg::ShapeDrawable(cone);
    coneDrawable->setColorArray(colors_red);
    coneDrawable->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::Geode> geode_arrow = new osg::Geode;
    geode_arrow->addDrawable(cylDrawable);
    geode_arrow->addDrawable(coneDrawable);
    axes_group->addChild(geode_arrow);

#if 1
    osg::ref_ptr<osg::Node> model_X = osgDB::readNodeFile(Core::findData("G4OSG","X.osgt"));
    if (!model_X)
      throw std::runtime_error("Problems loading X.osgt");
    auto geode_X = model_X->asGroup()->getChild(0)->asGroup()->getChild(0)->asGeode();
    if (!geode_X)
      throw std::runtime_error("could not get osg::Geode of letter X");
    auto billboard_X = dynamic_cast<osg::Billboard*>(geode_X);
    if (!billboard_X)
      throw std::runtime_error("could not get osg::Billboard of letter X");
    billboard_X->setAxis(osg::Vec3(1,0,0));
    billboard_X->setNormal(osg::Vec3(0,0,1));

    auto geo_X = dynamic_cast<osg::Geometry*>( geode_X->getDrawable(0) );
    if (!geo_X)
      throw std::runtime_error("could not get osg::Geometry of letter X");
    geo_X->setColorArray(colors_red);
    geo_X->setColorBinding(osg::Geometry::BIND_OVERALL);
    // if (!osgDB::writeNodeFile(*model_X.get(), "X_edited.osg"))
    //   throw std::runtime_error("problems writing X_edited.osg");
#else
    osg::ref_ptr<osg::Node> model_X = osgDB::readNodeFile(Core::findData("G4OSG","X_edited.osg"));
    if (!model_X)
      throw std::runtime_error("problems reading file");
#endif
    axes_group->addChild(model_X);
  }

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////// Y ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  {
    auto colors_green = new osg::Vec4Array;
    colors_green->push_back( osg::Vec4(0,1,0,1) );

    auto cyl_center = osg::Vec3( 0.0, cyl_offset, 0.0 );
    auto cone_center = osg::Vec3( 0.0, cone_offset, 0.0 );

    auto cylinder = new osg::Cylinder(cyl_center, cyl_radius, cyl_height);
    osg::Quat rot_z_to_y;
    rot_z_to_y.makeRotate(  -halfpi, 1.0, 0.0, 0.0 );
    cylinder->setRotation(rot_z_to_y);

    osg::ref_ptr<osg::ShapeDrawable> cylDrawable = new osg::ShapeDrawable(cylinder);
    cylDrawable->setColorArray(colors_green);
    cylDrawable->setColorBinding(osg::Geometry::BIND_OVERALL);
    auto cone = new osg::Cone(cone_center, cone_radius, cone_height);
    if ( cone->getBaseOffsetFactor() != cone_baseoffsetfactor )
      throw std::runtime_error("Unexpected osg::Cone BaseOffsetFactor");

    cone->setRotation(rot_z_to_y);

    osg::ref_ptr<osg::ShapeDrawable> coneDrawable = new osg::ShapeDrawable(cone);
    coneDrawable->setColorArray(colors_green);
    coneDrawable->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::Geode> geode_arrow = new osg::Geode;
    geode_arrow->addDrawable(cylDrawable);
    geode_arrow->addDrawable(coneDrawable);
    axes_group->addChild(geode_arrow);

#if 1
    osg::ref_ptr<osg::Node> model_Y = osgDB::readNodeFile(Core::findData("G4OSG","Y.osgt"));
    if (!model_Y)
      throw std::runtime_error("Problems loading Y.osgt");
    auto geode_Y = model_Y->asGroup()->getChild(0)->asGroup()->getChild(0)->asGeode();
    if (!geode_Y)
      throw std::runtime_error("could not get osg::Geode of letter Y");
    auto billboard_Y = dynamic_cast<osg::Billboard*>(geode_Y);
    if (!billboard_Y)
      throw std::runtime_error("could not get osg::Billboard of letter Y");
    billboard_Y->setAxis(osg::Vec3(0,1,0));
    billboard_Y->setNormal(osg::Vec3(0,0,1));
    auto geo_Y = dynamic_cast<osg::Geometry*>( geode_Y->getDrawable(0) );
    if (!geo_Y)
      throw std::runtime_error("could not get osg::Geometry of letter Y");
    geo_Y->setColorArray(colors_green);
    geo_Y->setColorBinding(osg::Geometry::BIND_OVERALL);
    // if (!osgDB::writeNodeFile(*model_Y.get(), "Y_edited.osg"))
    //   throw std::runtime_error("problems writing Y_edited.osg");
#else
    osg::ref_ptr<osg::Node> model_Y = osgDB::readNodeFile(Core::findData("G4OSG","Y_edited.osg"));
    if (!model_Y)
      throw std::runtime_error("problems reading file");
#endif
    axes_group->addChild(model_Y);
  }

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////// Z ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  {
    auto colors_blue = new osg::Vec4Array;
    colors_blue->push_back( osg::Vec4(0,0,1,1) );

    auto cyl_center = osg::Vec3( 0.0, 0.0, cyl_offset);
    auto cone_center = osg::Vec3(0.0,0.0,cone_offset);

    auto cylinder = new osg::Cylinder(cyl_center, cyl_radius, cyl_height);

    osg::ref_ptr<osg::ShapeDrawable> cylDrawable = new osg::ShapeDrawable(cylinder);
    cylDrawable->setColorArray(colors_blue);
    cylDrawable->setColorBinding(osg::Geometry::BIND_OVERALL);
    auto cone = new osg::Cone(cone_center, cone_radius, cone_height);
    if ( cone->getBaseOffsetFactor() != cone_baseoffsetfactor )
      throw std::runtime_error("Unexpected osg::Cone BaseOffsetFactor");

    osg::ref_ptr<osg::ShapeDrawable> coneDrawable = new osg::ShapeDrawable(cone);
    coneDrawable->setColorArray(colors_blue);
    coneDrawable->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::Geode> geode_arrow = new osg::Geode;
    geode_arrow->addDrawable(cylDrawable);
    geode_arrow->addDrawable(coneDrawable);
    axes_group->addChild(geode_arrow);

#if 1
    osg::ref_ptr<osg::Node> model_Z = osgDB::readNodeFile(Core::findData("G4OSG","Z.osgt"));
    if (!model_Z)
      throw std::runtime_error("Problems loading Z.osgt");
    auto geo_Z = dynamic_cast<osg::Geometry*>( model_Z->asGroup()->getChild(0)->asGroup()->getChild(0)->asGeode()->getDrawable(0) );
    if (!geo_Z)
      throw std::runtime_error("could not get osg::Geometry of letter Z");
    geo_Z->setColorArray(colors_blue);
    geo_Z->setColorBinding(osg::Geometry::BIND_OVERALL);
    // if (!osgDB::writeNodeFile(*model_Z.get(), "Z_edited.osg"))
    //   throw std::runtime_error("problems writing Z_edited.osg");
#else
    osg::ref_ptr<osg::Node> model_Z = osgDB::readNodeFile(Core::findData("G4OSG","Z_edited.osg"));
    if (!model_Z)
      throw std::runtime_error("problems reading file");
#endif
    axes_group->addChild(model_Z);
  }

  //if (!osgDB::writeNodeFile(*axes_group.get(), "dgcode_axes.osg"))
  //throw std::runtime_error("problems writing dgcode_axes.osg");

  return axes_group;
}
