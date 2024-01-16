#include "G4OSG/THCommon.hh"
#include "G4OSG/TrkHandle.hh"
#include "G4OSG/Viewer.hh"

G4OSG::THCommon::THCommon(Viewer* viewer,const char * griff_file, const std::set<std::uint64_t>& griff_evts_to_show)
  : m_viewer(viewer),
    m_dr(griff_file),
    m_griff_evts_to_show(griff_evts_to_show),
    m_sceneHook(new osg::Group),
    m_sceneHookHighlighted(new osg::Group),
    m_sceneHookNormal(new osg::Group),
    m_blockSelectionChangeCallbacks(false),
    m_style(99999),
    m_mode_aim(Viewer::envOption("G4OSG_DATAVIEW_AIMING_MODE")),
    m_mode_endtracksatvol(Viewer::envStr("G4OSG_ENDTRACKSATVOL")),
    m_mode_trkalpha(Viewer::envSet("G4OSG_TRKCOLALPHA")?Viewer::envInt("G4OSG_TRKCOLALPHA")*0.01:-1.0),
    m_mode_photonalpha(Viewer::envSet("G4OSG_PHOTONCOLALPHA")?Viewer::envInt("G4OSG_PHOTONCOLALPHA")*0.01:-1.0),
    m_mode_trkcolr(Viewer::envSet("G4OSG_TRKCOLR")?Viewer::envInt("G4OSG_TRKCOLR")*0.01:-1.0),
    m_mode_trkcolg(Viewer::envSet("G4OSG_TRKCOLG")?Viewer::envInt("G4OSG_TRKCOLG")*0.01:-1.0),
    m_mode_trkcolb(Viewer::envSet("G4OSG_TRKCOLB")?Viewer::envInt("G4OSG_TRKCOLB")*0.01:-1.0),
    m_mode_trklinewidth(Viewer::envSet("G4OSG_TRKLINEWIDTH")?Viewer::envDbl("G4OSG_TRKLINEWIDTH"):-1.0),
    m_mode_trkcolstyle(Viewer::envInt("G4OSG_TRKCOLSTYLE"))
{
  assert((m_mode_trkalpha==-1.0||(m_mode_trkalpha>=0.0&&m_mode_trkalpha<=1.0))&&"TRKCOLALPHA must be number in range 0..100");
  assert((m_mode_photonalpha==-1.0||(m_mode_photonalpha>=0.0&&m_mode_photonalpha<=1.0))&&"PHOTONCOLALPHA must be number in range 0..100");
  assert((m_mode_trkcolr==-1.0||(m_mode_trkcolr>=0.0&&m_mode_trkcolr<=1.0))&&"TRKCOLR must be number in range 0..100");
  assert((m_mode_trkcolg==-1.0||(m_mode_trkcolg>=0.0&&m_mode_trkcolg<=1.0))&&"TRKCOLG must be number in range 0..100");
  assert((m_mode_trkcolb==-1.0||(m_mode_trkcolb>=0.0&&m_mode_trkcolb<=1.0))&&"TRKCOLB must be number in range 0..100");
  assert((m_mode_trklinewidth==-1.0||(m_mode_trklinewidth>0.0&&m_mode_trklinewidth<=100.0))&&"TRKLINEWIDTH must be number in (0.0,100.0]");
  assert((m_mode_trkcolstyle>=0&&m_mode_trkcolstyle<=1)&&"TRKCOLSTYLE must be number in range 0..1 ");

  setStyle(0);
  //Create track-handles, one for each track identified by (evtidx,trkidx)

  bool ok(true);
  auto md = m_dr.setup()->metaData();
  auto itgm = md.find("GriffMode");
  if (itgm==md.end()){
    ok=false;
    printf("Viewer: ERROR: Griff file missing GriffMode meta-data!.\n");
  } else if (itgm->second == "REDUCED") {
    printf("Viewer: WARNING: Input Griff file is in REDUCED mode. FULL mode gives more detailed visualisation.\n");
  } else if (itgm->second == "MINIMAL") {
    ok=false;
    printf("Viewer: ERROR: Can't show Griff files in MINIMAL mode since step info is missing.\n");
  }

  if (ok) {
    unsigned nevt(0);
    unsigned nignored(0);
    bool skip_secondaries(Viewer::envOption("G4OSG_SKIPSECONDARIES"));

    while ( m_dr.loopEvents() ) {
      if ( !m_griff_evts_to_show.empty() && !m_griff_evts_to_show.count(m_dr.loopCount()) ) {
        ++nignored;
        continue;
      }
      ++nevt;
      unsigned evtidx = m_dr.eventIndexInCurrentFile();//same as loopCount for single input file
      unsigned ntracks = m_dr.nTracks();
      for (unsigned trkidx = 0; trkidx < ntracks; ++trkidx) {
        if (skip_secondaries&&!m_dr.getTrack(trkidx)->isPrimary())
          continue;
        if (!m_mode_aim || m_dr.getTrack(trkidx)->isPrimary())
          m_trkhandles.push_back(new TrkHandle(this,trkidx,evtidx));
      }
    }
    printf("Viewer: Loaded %i Griff events with %i tracks (%g tracks/evt)\n",
           nevt,(int)m_trkhandles.size(),
           (nevt?double(m_trkhandles.size())/nevt:0.0));
    if (nignored)
      printf("Viewer: WARNING: Skipped %u events in file due to event selection\n",nignored);
    if (m_trkhandles.empty())
      printf("Viewer: WARNING: No tracks to display!\n");
  }
}

G4OSG::THCommon::~THCommon()
{
#if 0//The code here avoids memory leaks, but slows down program shut-down for no reason.
  auto itE = m_trkhandles.end();
  for (auto it = m_trkhandles.begin();it!=itE;++it) {
    (*it)->hide();
    delete *it;
  }
#endif
}

void G4OSG::THCommon::setStyle(unsigned style)
{
  //For now, just 1 style (keeping code here for now, for possible additions)
  assert(style<1);
  if (style==m_style)
    return;

  m_viewer->safeStopThreading();

  //deinstall old style:
  if (m_style==0) {
    m_sceneHook->removeChild(m_sceneHookNormal);
  }
  //install new style:
  m_style = style;
  if (m_style==0) {
    m_sceneHook->addChild(m_sceneHookNormal);
  }

  m_viewer->safeStartThreading();
}

void G4OSG::THCommon::clearAllSelectedHandles()
{
  bool store = m_blockSelectionChangeCallbacks;
  m_blockSelectionChangeCallbacks=true;

  auto itE = m_selectedHandles.end();
  for (auto it = m_selectedHandles.begin();it!=itE;++it)
    (*it)->setHighlighted(false);
  m_selectedHandles.clear();

  m_blockSelectionChangeCallbacks=store;
  selectionMightHaveChanged();
}

void G4OSG::THCommon::toggleSelectHandle(TrkHandle*th)
{
  bool store = m_blockSelectionChangeCallbacks;
  m_blockSelectionChangeCallbacks=true;

  auto it = m_selectedHandles.find(th);
  if (it==m_selectedHandles.end())
    selectHandle(th);
  else {
    if (m_selectedHandles.size()>=2) {
      clearAllSelectedHandles();
      selectHandle(th);
    } else {
      deselectHandle(th);
    }
  }
  m_blockSelectionChangeCallbacks=store;
  selectionMightHaveChanged();
}

bool G4OSG::THCommon::handleIsSelected(const TrkHandle*th) const
{
  return m_selectedHandles.find(const_cast<TrkHandle*>(th))!=m_selectedHandles.end();
}

void G4OSG::THCommon::selectHandle(TrkHandle*th)
{
  bool store = m_blockSelectionChangeCallbacks;
  m_blockSelectionChangeCallbacks=true;

  clearAllSelectedHandles();

  m_selectedHandles.insert(th);
  th->setHighlighted(true);

  m_blockSelectionChangeCallbacks=store;
  selectionMightHaveChanged();
}

void G4OSG::THCommon::deselectHandle(TrkHandle*th)
{
  bool store = m_blockSelectionChangeCallbacks;
  m_blockSelectionChangeCallbacks=true;

  clearAllSelectedHandles();

  m_selectedHandles.erase(th);
  th->setHighlighted(false);

  m_blockSelectionChangeCallbacks=store;
  selectionMightHaveChanged();
}


void G4OSG::THCommon::selectionMightHaveChanged()
{
  if (m_blockSelectionChangeCallbacks)
    return;
  if (m_selectedHandlesLastCB==m_selectedHandles)
    return;
  m_selectedHandlesLastCB=m_selectedHandles;

  printf("TRACK SELECTION CHANGED CALLBACK\n");//fixme

  //fixme, for now we just print a bit of info here rather than emit a callback:

  printf("#############################################\n");
  printf("################ SELECTION ##################\n");

  for (auto it=m_selectedHandles.begin();it!=m_selectedHandles.end();++it) {
    printf("### lala fixme (track info)\n");
    printf("-----------------------------------------------------------\n");
    printf("\n");
  }
  printf("#############################################\n");
  printf("#############################################\n");
}

void G4OSG::THCommon::displayAllTracks()
{
  auto itE = m_trkhandles.end();
  for (auto it = m_trkhandles.begin();it!=itE;++it) {
    (*it)->display();
  }
}

bool G4OSG::THCommon::getExtent( const std::vector<TrkHandle*>& trks,
                                 double&xmin, double&xmax,
                                 double&ymin, double&ymax,
                                 double&zmin, double&zmax)
{
  if (trks.empty())
    return false;
  auto it = trks.begin();
  auto itE = trks.end();
  (*it)->getExtent(xmin,xmax,ymin,ymax,zmin,zmax);
  double x0,x1,y0,y1,z0,z1;
  for (++it;it!=itE;++it) {
    if (!(*it)->getExtent(x0,x1,y0,y1,z0,z1))
      return false;
    xmin = std::min<double>(xmin,x0);
    ymin = std::min<double>(ymin,y0);
    zmin = std::min<double>(zmin,z0);
    xmax = std::max<double>(xmax,x1);
    ymax = std::max<double>(ymax,y1);
    zmax = std::max<double>(zmax,z1);
  }
  return true;
}
