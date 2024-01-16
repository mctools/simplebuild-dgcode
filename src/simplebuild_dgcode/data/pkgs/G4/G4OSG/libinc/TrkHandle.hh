#ifndef G4OSG_TrkHandle_hh
#define G4OSG_TrkHandle_hh

#include "G4OSG/THCommon.hh"
#include <osg/Geode>
#include <vector>
#include <cassert>
#include "GriffDataRead/GriffDataReader.hh"

namespace G4OSG {

  class TrkHandle;

  class THHolder : public osg::Referenced {
  public:
    THHolder(TrkHandle*th) : m_th(th) {}
    ~THHolder(){}
    TrkHandle * trkHandle() const { return m_th; }
  private:
    TrkHandle * m_th;
  };

  class TrkHandle {//todo: inherit from VisHandle base class (common for geo, particles, ...)
  private:
    friend class THCommon;
    TrkHandle( const TrkHandle & );
    TrkHandle & operator= ( const TrkHandle & );
    TrkHandle(THCommon*,unsigned trkIdxInEvt,unsigned evtIdxInFile);
    ~TrkHandle();

  public:

    //control whether volume should be displayed or hidden:
    void setDisplayed(bool t);
    void display();
    void hide();
    bool isDisplayed() const;
    bool isRendered() { return isDisplayed(); }

    //control whether volume should be highlighted (when not hidden of course)
    void setHighlighted(bool);
    bool isHighlighted() const;

    //Get the Griff track (NB: This might cause event seeking in the Griff file, invalidating other retained Griff tracks!!)
    const GriffDataRead::Track * griffTrack() const;

    const osg::ref_ptr<osg::Geode> osgGeode() const;
    void setAlpha(double a) const {m_colors->back()[3] = a; }

    bool getExtent( double&xmin, double&xmax,
                    double&ymin, double&ymax,
                    double&zmin, double&zmax);//brute force bounding box (for auto scene positioning)

    std::string getHoverInfo(unsigned istep) const;

  private:
    THCommon * m_common;
    mutable osg::Vec4Array* m_colors;
    unsigned m_trkIdxInEvt;
    unsigned m_evtIdxInFile;
    bool m_displayed;
    bool m_highlighted;
    mutable osg::ref_ptr<osg::Geode> m_osgGeode;
    mutable osg::Vec3Array* m_vertexArray;
    void setupGeode() const;
  };

}

#include "G4OSG/TrkHandle.icc"

#endif
