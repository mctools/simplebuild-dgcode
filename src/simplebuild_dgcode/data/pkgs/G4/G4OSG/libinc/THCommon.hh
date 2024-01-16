#ifndef G4OSG_THCommon_hh
#define G4OSG_THCommon_hh

//All selection/highlight stuff is leftovers, that we might use soon.

#include <osg/Group>

#include "GriffDataRead/GriffDataReader.hh"

namespace G4OSG {

  class TrkHandle;
  class Viewer;

  class THCommon : public osg::Referenced {
  public:
    //Show tracks in griff file. Empty griff_evts_to_show means all, otherwise
    //just show events at those positions in the file.
    THCommon(Viewer*, const char * griff_file, const std::set<std::uint64_t>& griff_evts_to_show);
    ~THCommon();
    osg::Group * sceneHook() { return m_sceneHook; }

    //Nodes representing volumes attach themselves under one of these, depending
    //on whether or not they are highlighted:
    osg::Group * sceneHookHighlighted() { return m_sceneHookHighlighted; }
    osg::Group * sceneHookNormal() { return m_sceneHookNormal; }

    void nextDisplayStyle() { setStyle((m_style+1)%4); }
    void previousDisplayStyle() { setStyle((m_style+3)%4); }
    void setStyle(unsigned style);

    std::set<TrkHandle*>& selectedHandles() { return m_selectedHandles; }
    void clearAllSelectedHandles();
    void toggleSelectHandle(TrkHandle*);
    bool handleIsSelected(const TrkHandle*) const;
    void selectHandle(TrkHandle*);
    void deselectHandle(TrkHandle*);

    //Something with "show single event (the one of the selected or the first), or all trks from all events
    //    void resetGeometry();//show just the volumes below the world
    void displayAllTracks();

    GriffDataReader* griffDataReader() { return &m_dr; }

    std::vector<TrkHandle*>& trkHandles() { return m_trkhandles; }

    bool aimMode() const { return m_mode_aim; }

    //get extent of group of tracks:
    static bool getExtent( const std::vector<TrkHandle*>&,
                           double&xmin, double&xmax,
                           double&ymin, double&ymax,
                           double&zmin, double&zmax);

    const std::string& modeEndTracksAtVolName() const { return m_mode_endtracksatvol; }//empty means not enabled
    //-1.0 in the next methods means not enabled:
    double modeTrkAlpha() const { return m_mode_trkalpha; }
    double modePhotonAlpha() const { return m_mode_photonalpha; }
    double modeTrkColR() const { return m_mode_trkcolr; }
    double modeTrkColG() const { return m_mode_trkcolg; }
    double modeTrkColB() const { return m_mode_trkcolb; }
    double modeTrkLineWidth() const { return m_mode_trklinewidth; }
    int modeTrkColorStyle() const { return m_mode_trkcolstyle; }

  private:
    Viewer * m_viewer;
    GriffDataReader m_dr;
    std::set<std::uint64_t> m_griff_evts_to_show;
    std::vector<TrkHandle*> m_trkhandles;
    //    TrkHandle* m_vhWorld;
    osg::ref_ptr<osg::Group> m_sceneHook;
    osg::ref_ptr<osg::Group> m_sceneHookHighlighted;
    osg::ref_ptr<osg::Group> m_sceneHookNormal;

    std::set<TrkHandle*> m_selectedHandles;//FIXME: Sort by e.g. (name,matname,copynbr,ptr) rather than pointers
    std::set<TrkHandle*> m_selectedHandlesLastCB;
    //std::list<TrkHandle*> m_zappedHandles;
    void selectionMightHaveChanged();
    bool m_blockSelectionChangeCallbacks;

    // //styles:
    unsigned m_style;
    // osg::ref_ptr<osgFX::Scribe> m_scribeEffectHighlighted;
    // osg::ref_ptr<osgFX::Scribe> m_scribeEffectNormal;
    // osg::ref_ptr<osgFX::Cartoon> m_cartoonEffectHighlighted;
    // osg::ref_ptr<osgFX::Cartoon> m_cartoonEffectNormal;
    // osg::ref_ptr<osgFX::Outline> m_outlineEffectHighlighted;
    // osg::ref_ptr<osgFX::Outline> m_outlineEffectNormal;

    //to be used by the VolHandle's:
    // friend class VolHandle;
    // void registerZap(VolHandle*vh);
    // void registerUnzap(VolHandle*vh);
    bool m_mode_aim;
    std::string m_mode_endtracksatvol;
    double m_mode_trkalpha;
    double m_mode_photonalpha;
    double m_mode_trkcolr;
    double m_mode_trkcolg;
    double m_mode_trkcolb;
    double m_mode_trklinewidth;
    int m_mode_trkcolstyle;

  };
}

#endif
