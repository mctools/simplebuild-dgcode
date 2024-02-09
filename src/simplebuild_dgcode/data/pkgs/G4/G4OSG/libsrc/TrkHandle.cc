#include "G4OSG/TrkHandle.hh"
#include "G4OSG/ObjectHolder.hh"
#include <stdexcept>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <sstream>
#include "Utils/NeutronMath.hh"
#include "G4OSG/BestUnit.hh"

G4OSG::TrkHandle::~TrkHandle()
{
}

const GriffDataRead::Track * G4OSG::TrkHandle::griffTrack() const
{
  auto dr = m_common->griffDataReader();
  //Spool to right event:
  if (!dr->eventActive() || dr->eventIndexInCurrentFile() != m_evtIdxInFile) {
    bool seek = dr->seekEventByIndexInCurrentFile(m_evtIdxInFile);
    if (!seek)
      throw std::runtime_error("G4OSG::TrkHandle::griffTrack Event seek error!");
    assert(seek);
    assert(dr->eventIndexInCurrentFile() == m_evtIdxInFile);
    assert(dr->eventActive());
  }
  //Return the track (NB - only valid until someone else spools, for instance by
  //calling the present method!!):
  auto trk = dr->getTrack(m_trkIdxInEvt);
  assert(trk);
  return trk;
}

void G4OSG::TrkHandle::setupGeode() const
{
  assert(!m_osgGeode.valid());

  bool aim_mode(m_common->aimMode());

  // create drawable geometry object
  osg::Geometry* geo = new osg::Geometry;//LEAK

  // add vertices
  osg::Vec3Array* verts = new osg::Vec3Array;//LEAK
  m_vertexArray = verts;
  // and a primitive set (add index numbers)
  osg::DrawElementsUInt* ps =
    new osg::DrawElementsUInt( osg::PrimitiveSet::LINES, 0 );

  unsigned npoints(0);
  auto trk = griffTrack();
  assert(trk);

  const int trkcolstyle = m_common->modeTrkColorStyle();

  auto trkcolor = osg::Vec4(1,1,1,0);//default color
  if (aim_mode) {
    if (trk->nSegments()>1) trkcolor = osg::Vec4(1,0,0,0);
    else trkcolor = osg::Vec4(0.5,0.5,0,0);
  } else {
    //colour by particle type:
    int pdg=trk->pdgCode();
    if (trkcolstyle==0) {
      if (pdg==22) trkcolor = osg::Vec4(1,1,0,0);//photon
      else if (pdg==2112) trkcolor = osg::Vec4(0,1,0,0);//neutron
      else if (pdg==2212) trkcolor = osg::Vec4(1,0,0,0);//proton
      else if (pdg==11) trkcolor = osg::Vec4(0,0,1,0);//electron
      else if (pdg==211||pdg==-211) trkcolor = osg::Vec4(1,0,1,0);//pi+/pi-
      else if(pdg==1000020040) trkcolor = osg::Vec4(0,1,1,0); // alpha
      else if(pdg==1000030070) trkcolor = osg::Vec4(1,0.27,0,0); // Li7
    } else {
      assert(trkcolstyle==1);
      // #colors inspired by http://www.mulinblog.com/a-color-palette-optimized-for-data-visualization/
      // Used for paper...
      trkcolor = osg::Vec4(0.301960784314,0.301960784314,0.301960784314,0); //change default to gray
      if (pdg==22) trkcolor = osg::Vec4(0.8*0.870588235294,0.8*0.811764705882,0.8*0.247058823529,0);//photon [dark YELLOW, scaled from yellow]
      else if (pdg==2112) trkcolor = osg::Vec4(0.376470588235,0.741176470588,0.407843137255,0);//neutron [GREEN]
      else if (pdg==2212) trkcolor = osg::Vec4(0.945098039216,0.345098039216,0.329411764706,0);//proton [RED]
      else if (pdg==11) trkcolor = osg::Vec4(0.364705882353,0.647058823529,0.854901960784,0);//electron [BLUE]
      else if (pdg==211||pdg==-211) trkcolor = osg::Vec4(0.698039215686,0.462745098039,0.698039215686,0);//pi+/pi- [PURPLE]
      else if(pdg==1000020040) trkcolor = osg::Vec4(0,1,1,0); // alpha [cyan, not part of colour palette.
      else if(pdg==1000030070) trkcolor = osg::Vec4(0.980392156863,0.643137254902,0.227450980392,0); // Li7 [ORANGE]
      else if (pdg==-11) trkcolor = osg::Vec4(0.7*0.364705882353,0.7*0.647058823529,0.7*0.854901960784,0);//positron [dark blue, scaled from blue]
      //Unused:
      //(0.698039215686,0.56862745098,0.18431372549,0) : brown
      //(0.945098039216,0.486274509804,0.690196078431,0) : pink
    }
  }
  if (m_common->modeTrkColR()!=-1.0) trkcolor[0] = m_common->modeTrkColR();
  if (m_common->modeTrkColG()!=-1.0) trkcolor[1] = m_common->modeTrkColG();
  if (m_common->modeTrkColB()!=-1.0) trkcolor[2] = m_common->modeTrkColB();

  //todo: should we .reserve() the arrays?

  bool endTracksPrematurely(!m_common->modeEndTracksAtVolName().empty());
  auto segE= trk->segmentEnd();
  const GriffDataRead::Step * step(0);
  const GriffDataRead::Step * lastStep(0);
  for (auto seg = trk->segmentBegin(); seg!=segE; ++seg) {
    assert(seg);
    if (endTracksPrematurely && m_common->modeEndTracksAtVolName() == seg->volumeName())
      break;
    auto stepE = seg->stepEnd();
    for (step = seg->stepBegin(); step!=stepE; ++step) {
      assert(step);
      verts->push_back( osg::Vec3( step->preGlobalX(), step->preGlobalY(), step->preGlobalZ() ) );
      ps->push_back( npoints );
      ps->push_back( ++npoints );
      lastStep = step;
    }
    if (aim_mode)
      break;
  }
  assert(npoints&&step);

  //Just missing the end of the last step:
  if ( lastStep )
    verts->push_back( osg::Vec3( lastStep->postGlobalX(), lastStep->postGlobalY(), lastStep->postGlobalZ() ) );

  geo->setVertexArray( verts );
  geo->addPrimitiveSet( ps );

  m_colors = new osg::Vec4Array;
  m_colors->push_back(trkcolor);
  geo->setColorArray(m_colors);
  geo->setColorBinding(osg::Geometry::BIND_OVERALL);
  setAlpha(1.0);

  // create geometry node that will contain all our drawables
  m_osgGeode = new osg::Geode;//LEAK
  osg::StateSet* pStateSet = m_osgGeode->getOrCreateStateSet();//LEAK
  pStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
  pStateSet->setMode( GL_LINE_SMOOTH, osg::StateAttribute::ON );
  pStateSet->setMode( GL_BLEND, osg::StateAttribute::ON );
  if (m_common->modeTrkLineWidth()!=-1.0) {
    osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth;
    lineWidth->setWidth(m_common->modeTrkLineWidth());
    pStateSet->setAttribute( lineWidth.get() );
  }
  m_osgGeode->addDrawable( geo );
  assert(m_osgGeode.valid());

  //track alpha, depending on number of tracks:
  double alpha = 1.0;
  double overlap_scale = sqrt((double)m_common->trkHandles().size());
  if (overlap_scale>30)
    alpha *= 30.0/overlap_scale;
  if (alpha<0.3)
    alpha=0.3;
  setAlpha(alpha);

  if ( m_common->modeTrkAlpha()!=-1.0 )
    setAlpha(m_common->modeTrkAlpha());//env overrode alpha
  if ( m_common->modePhotonAlpha()!=-1.0 && trk->pdgCode()==22 )
    setAlpha(m_common->modePhotonAlpha());//env overrode alpha of photons

  m_osgGeode->setUserData(new ObjectHolder((void*)this,2));

}

void G4OSG::TrkHandle::display()
{
  if (m_displayed)
    return;
  assert(!isRendered());
  m_displayed=true;
  if (isRendered()) {
    if (isHighlighted()) {
      m_common->sceneHookHighlighted()->addChild(osgGeode());
    } else {
      m_common->sceneHookNormal()->addChild(osgGeode());//LEAK
    }
  }
}

void G4OSG::TrkHandle::hide()
{
  if (!m_displayed)
    return;

  if (isRendered()) {
    assert(m_osgGeode.valid());
    if (isHighlighted())
      m_common->sceneHookHighlighted()->removeChild(osgGeode());
    else
      m_common->sceneHookNormal()->removeChild(osgGeode());
  }
  m_displayed=false;
  assert(!isRendered());

  //hiding a volume deselects it: (fixme: only if selected!!)
  m_common->deselectHandle(this);
}

void G4OSG::TrkHandle::setHighlighted(bool hl)
{
  if (m_highlighted == hl)
    return;

  m_highlighted = hl;

  if (!isRendered())
    return;

  if (hl) {
    //rendered volume going from not highlighted to highlighted
    m_common->sceneHookNormal()->removeChild(osgGeode());
    m_common->sceneHookHighlighted()->addChild(osgGeode());
  } else {
    //rendered volume going from highlighted to not highlighted
    m_common->sceneHookHighlighted()->removeChild(osgGeode());
    m_common->sceneHookNormal()->addChild(osgGeode());
  }
}

bool G4OSG::TrkHandle::getExtent( double&xmin, double&xmax,
                                  double&ymin, double&ymax,
                                  double&zmin, double&zmax)
{
  //TODO: here we might trigger the geode creation, we could consider doing it
  //directly from Griff steps.
  if (!m_vertexArray)
    osgGeode();
  assert (m_vertexArray);
  if (m_vertexArray->size()<2)
    return false;
  auto itE = m_vertexArray->end();
  auto it = m_vertexArray->begin();
  xmin = xmax = it->x();
  ymin = ymax = it->y();
  zmin = zmax = it->z();
  for (++it;it!=itE;++it) {
    xmin = std::min<double>(xmin,it->x());
    ymin = std::min<double>(ymin,it->y());
    zmin = std::min<double>(zmin,it->z());
    xmax = std::max<double>(xmax,it->x());
    ymax = std::max<double>(ymax,it->y());
    zmax = std::max<double>(zmax,it->z());
  }
  return true;
}

std::string G4OSG::TrkHandle::getHoverInfo(unsigned istep) const
{
  //Find trk, segment and step:
  auto trk = griffTrack();
  auto segE = trk->segmentEnd();
  auto seg = trk->segmentBegin();
  const GriffDataRead::Step * step(0);
  for (;seg!=segE;++seg) {
    if (istep<seg->nStepsStored()) {
      step  = seg->getStep(istep);
      break;
    } else {
      istep -= seg->nStepsStored();
    }
  }
  if (seg==segE||!step)
    return "Track <error accessing data>";

  std::stringstream ss;

  double wavelength(-1);
  double mass = trk->mass()/Constants::c_squared;
  double ekin = step->preEKin();
  if (ekin==0) {
    wavelength = -1;//don't show
  } else {
    if (mass) {
      wavelength = Constants::h_Planck*sqrt(0.5/(mass*ekin));
    } else {
      wavelength = Constants::h_Planck*Constants::c_light/ekin;
    }
  }

  if (trk->pdgCode()!=2112&&trk->pdgCode()!=22)
    wavelength = -1;
  ss << "Track "<<trk->pdgName()<<" [";
  //  ss<<"evt "<<trk->getDataReader()->eventIndexInCurrentFile();
  if (trk->isPrimary()) {
    ss<<"primary particle";
  } else {
    ss<<"daughter of ";
    auto parent = trk->getParent();
    if (parent)
      ss << parent->pdgName();
    else
      ss << "trk"<<trk->parentID()<<"<notstored>";
    ss<<" via \""<<trk->creatorProcess()<<"\"";
  }

  ss << ", "<<G4OSG::BestUnit(ekin, "Energy");
  if (wavelength>-1)
    ss << " ("<<G4OSG::BestUnit(wavelength, "Length")<<")";
  if (trk->weight()!=1.0)
    ss << ", weight "<<trk->weight();
  if (trk->nDaughters())
    ss<<", "<<trk->nDaughters()<<" daughter"<<(trk->nDaughters()==1?"":"s");
  ss<<"]";
  return ss.str();
}
