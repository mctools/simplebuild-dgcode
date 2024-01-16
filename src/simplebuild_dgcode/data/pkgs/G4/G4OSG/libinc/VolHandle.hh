#ifndef G4OSG_VolHandle_hh
#define G4OSG_VolHandle_hh

#include "G4OSG/VHCommon.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <vector>
#include <cassert>

namespace G4OSG {

  class VolHandle;
  class HUD;

  class VolHandle {//todo: inherit from VisHandle base class (common for geo, particles, ...)
  private:
    friend class VHCommon;
    VolHandle( const VolHandle & );
    VolHandle & operator= ( const VolHandle & );
    VolHandle(VHCommon*,const G4VPhysicalVolume *,VolHandle * mother = 0);
    ~VolHandle();

  public:

    //navigation stuff returning mother/daughter are not const, as they provide non-const pointer back to us):
    VolHandle * mother() const;
    unsigned nDaughters() const;
    VolHandle * daughter(unsigned i);
    VolHandle * * daughtersBegin();
    VolHandle * * daughtersEnd();

    //NB: Displayed-hidden state is independent from zapped-unzapped state.

    //control whether volume should be displayed or hidden:
    void setDisplayed(bool t);
    void display();
    void hide();
    bool isDisplayed() const;

    //control whether a volume should be zapped:
    void setZapped( bool z = true ) { if (z) { zap(); } else { unzap(); } }
    void zap();
    void unzap();
    bool isZapped() const { return m_zapped; }

    //A volume is only actually rendered when it is displayed and unzapped"
    bool isRendered() { return isDisplayed() && !isZapped(); }

    //control the display style of volume when zapped:
    enum DISPLAYSTYLE { NORMAL, WIREFRAME, TRANSPARENT };
    DISPLAYSTYLE displayStyle() { return m_displayStyle; }
    void setDisplayStyle(DISPLAYSTYLE);
    void setDisplayStyleRecursively(DISPLAYSTYLE);
    //how to make mother "zapped" and open?
    //Ways either kill a volume or expand it (the two first are the old-school ones):
    //      "zap, expanded-invisible, expanded-wireframe, expanded-transparent"

    //control whether volume should be highlighted (when not hidden of course)
    void setHighlighted(bool);
    bool isHighlighted() const;

    const G4VPhysicalVolume * g4PhysVol() const;
    const G4LogicalVolume * g4LogVol() const;
    const G4VSolid * g4Solid() const;
    std::string pvNameMaxLength(unsigned) const;
    std::string lvNameMaxLength(unsigned) const;
    std::string solidNameMaxLength(unsigned) const;
    std::string matNameMaxLength(unsigned) const;
    bool g4MarkedInvisible() const;

    const osg::ref_ptr<osg::MatrixTransform> osgTrf() const;//trf node which as a daughter has the geode
    const osg::ref_ptr<osg::Geode> osgGeode() const;
    const osg::Matrixd& getMatrix() const { return osgTrf()->getMatrix(); }

    bool findNearestCorner(const osg::Vec3d& point,osg::Vec3d& corner, double& dist_to_corner, bool local_coords = false);
    bool findNormal(const osg::Vec3d& point,osg::Vec3d& normal, bool local_coords = false);

    bool getExtent( double&xmin, double&xmax,
                    double&ymin, double&ymax,
                    double&zmin, double&zmax);//brute force bounding box (for auto scene positioning)

    bool getExtentDaughters( double&xmin, double&xmax,
                             double&ymin, double&ymax,
                             double&zmin, double&zmax);//Combined extent of daughter vols

    void displayDaughters();
    void hideDaughters();
    void hideDaughtersRecursively();
    bool hasVisibleDaughtersRecursively();
    void updateRotationalShapesRecursively();

    //Not really debugged:
    bool rayIntersection(const osg::Vec3d&p,
                         const osg::Vec3d&d,
                         osg::Vec3d& intersectionPoint,
                         osg::Vec3d& intersectionNormal);

    //change visibility/transparency,colour,rendering mode, ...
    //picked call-back!
    //stuff to return shape, transformation, ...
    //zoom to volume (external function perhaps - or a method on the viewer?)

    std::string getHoverInfo() const;

  private:
    void nameML(const std::string& in, std::string& out, unsigned) const;
    VHCommon * m_common;
    const G4VPhysicalVolume * m_physVol;
    VolHandle * m_motherHandle;
    bool m_displayed;
    bool m_zapped;
    bool m_highlighted;
    DISPLAYSTYLE m_displayStyle;

    mutable std::vector<VolHandle *> m_daughterHandles;
    mutable bool m_daughterInit;
    mutable osg::ref_ptr<osg::MatrixTransform> m_osgTrf;
    mutable osg::ref_ptr<osg::Geode> m_osgGeode;

    void setupTrf() const;
    void setupGeode() const;
    void initDaughters() const;
  };

}

#include "G4OSG/VolHandle.icc"

#endif
