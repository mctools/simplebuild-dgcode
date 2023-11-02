#ifndef G4OSG_G4SolidConverter_hh
#define G4OSG_G4SolidConverter_hh

// Converts G4VSolid objects to osg::Geode objects. For some basic G4 shapes
// (box, sphere, tube, ...) the proper corresponding OSG drawable will be used,
// and for others it will fallback to use the polyhedronisation provided by G4.

//TODO: Figure out how to deal with the "number of phi sectors" properly, so it can be changed on demand in the gui

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/ref_ptr>
class G4VSolid;

namespace G4OSG {
  osg::ref_ptr<osg::Geometry> g4solid2Geometry(const G4VSolid*);
  osg::ref_ptr<osg::Geode> g4solid2Geode(const G4VSolid*);
}

#endif
