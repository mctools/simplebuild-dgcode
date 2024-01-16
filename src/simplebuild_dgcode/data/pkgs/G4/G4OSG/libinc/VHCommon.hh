#ifndef G4OSG_VHCommon_hh
#define G4OSG_VHCommon_hh

#include <osg/Group>
#include <osgFX/Scribe>
#include <osgFX/Cartoon>
// #include <osgFX/Outline>
#include <vector>

class G4VPhysicalVolume;

namespace G4OSG {

  class VolHandle;
  class Viewer;

  class VHCommon : public osg::Referenced {
  public:
    VHCommon(Viewer*, const G4VPhysicalVolume * world);
    ~VHCommon();
    osg::Group * sceneHook() { return m_sceneHook; }

    //Nodes representing volumes attach themselves under one of these, depending
    //on whether or not they are highlighted:
    osg::Group * sceneHookHighlighted() { return m_sceneHookHighlighted; }
    osg::Group * sceneHookNormal() { return m_sceneHookNormal; }

    void nextDisplayStyle() { setStyle((m_style+1)%4); }
    void previousDisplayStyle() { setStyle((m_style+3)%4); }
    void setStyle(unsigned style);

    VolHandle * volHandleWorld() { return m_vhWorld; }

    unsigned getNPhiRendered() const;
    void setNPhiRendered(unsigned);

    std::set<VolHandle*>& selectedHandles() { return m_selectedHandles; }
    void clearAllSelectedHandles();
    void toggleSelectHandle(VolHandle*,bool multiselect=false);
    bool handleIsSelected(const VolHandle*) const;
    void selectHandle(VolHandle*,bool multiselect=false);
    void deselectHandle(VolHandle*,bool multiselect=false);

    void requestExtendedInfo();

    void unzapPrevious();
    void unzapAll();
    void resetGeometry();//show just the volumes below the world

  private:

    Viewer * m_viewer;
    VolHandle* m_vhWorld;
    osg::ref_ptr<osg::Group> m_sceneHook;
    osg::ref_ptr<osg::Group> m_sceneHookHighlighted;
    osg::ref_ptr<osg::Group> m_sceneHookNormal;

    std::set<VolHandle*> m_selectedHandles;//FIXME: Sort by e.g. (name,matname,copynbr,ptr) rather than pointers
    std::set<VolHandle*> m_selectedHandlesLastCB;
    std::list<VolHandle*> m_zappedHandles;
    void selectionMightHaveChanged();
    bool m_blockSelectionChangeCallbacks;

    //styles:
    unsigned m_style;
    osg::ref_ptr<osgFX::Scribe> m_scribeEffectHighlighted;
    osg::ref_ptr<osgFX::Scribe> m_scribeEffectNormal;
    osg::ref_ptr<osgFX::Cartoon> m_cartoonEffectHighlighted;
    osg::ref_ptr<osgFX::Cartoon> m_cartoonEffectNormal;
    // osg::ref_ptr<osgFX::Outline> m_outlineEffectHighlighted;
    // osg::ref_ptr<osgFX::Outline> m_outlineEffectNormal;

    //to be used by the VolHandle's:
    friend class VolHandle;
    void registerZap(VolHandle*vh);
    void registerUnzap(VolHandle*vh);
  };
}

#endif
