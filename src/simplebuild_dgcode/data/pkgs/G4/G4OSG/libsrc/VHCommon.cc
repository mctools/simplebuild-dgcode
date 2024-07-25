#include "G4OSG/VHCommon.hh"
#include "G4OSG/VolHandle.hh"
#include "G4OSG/Viewer.hh"
#include "HepPolyhedron.h"
#include "G4OSG/BestUnit.hh"
#include <sstream>

G4OSG::VHCommon::VHCommon(Viewer* viewer,const G4VPhysicalVolume*world)
  : m_viewer(viewer),
    m_sceneHook(new osg::Group),
    m_sceneHookHighlighted(new osg::Group),
    m_sceneHookNormal(new osg::Group),
    m_blockSelectionChangeCallbacks(false),
    m_style(99999),
    m_scribeEffectHighlighted(new osgFX::Scribe),//this one is also used to highlight in "raw" mode.
    m_scribeEffectNormal(new osgFX::Scribe),
    m_cartoonEffectHighlighted(new osgFX::Cartoon),
    m_cartoonEffectNormal(new osgFX::Cartoon)
    //fixme: detect availability of Outline at startup:
    // m_outlineEffectHighlighted(new osgFX::Outline),
    // m_outlineEffectNormal(new osgFX::Outline)
{
  assert(world);
  m_vhWorld = new VolHandle(this,world);
  setStyle(1);
}

G4OSG::VHCommon::~VHCommon()
{
  delete m_vhWorld;
}

void G4OSG::VHCommon::setStyle(unsigned style)
{
  //0 : no FX
  //1 : scribe with outline color1
  //2 : scribe with outline color2
  //3 : cartoon
  //4 : outline [DISABLED]


  assert(style<4);
  if (style==m_style)
    return;

  m_viewer->safeStopThreading();

  //various settings:
  auto highlight_color = osg::Vec4(1.0,0.0,0.0,1.0);
  const double highlight_linewidth = 5.0;
  auto scribe_color1 = osg::Vec4(0.2,0.2,0.2,1.0);
  auto scribe_color2 = osg::Vec4(0.8,0.8,0.8,1.0);
  auto cartoon_color1 = osg::Vec4(0.0,0.0,0.0,1.0);
  // auto outline_color1 = scribe_color1;

  //deinstall old style:
  if (m_style==0) {
    //raw (using scribe for the highlighting)
    m_scribeEffectHighlighted->removeChild(m_sceneHookHighlighted);
    m_sceneHook->removeChild(m_scribeEffectHighlighted);
    m_sceneHook->removeChild(m_sceneHookNormal);
  } else if (m_style==1||m_style==2) {
    //deinstall scribe
    m_scribeEffectHighlighted->removeChild(m_sceneHookHighlighted);
    m_sceneHook->removeChild(m_scribeEffectHighlighted);
    m_scribeEffectNormal->removeChild(m_sceneHookNormal);
    m_sceneHook->removeChild(m_scribeEffectNormal);
  } else if (m_style==3) {
    //deinstall cartoon
    m_cartoonEffectHighlighted->removeChild(m_sceneHookHighlighted);
    m_sceneHook->removeChild(m_cartoonEffectHighlighted);
    m_cartoonEffectNormal->removeChild(m_sceneHookNormal);
    m_sceneHook->removeChild(m_cartoonEffectNormal);
  }
  // } else if (m_style==4) {
  //   //deinstall outline
  //   m_outlineEffectHighlighted->removeChild(m_sceneHookHighlighted);
  //   m_sceneHook->removeChild(m_outlineEffectHighlighted);
  //   m_outlineEffectNormal->removeChild(m_sceneHookNormal);
  //   m_sceneHook->removeChild(m_outlineEffectNormal);
  // }
  m_style = style;
  if (m_style==0||m_style==1||m_style==2) {
    //install scribe-highlights:
    m_scribeEffectHighlighted->setWireframeColor(highlight_color);
    m_scribeEffectHighlighted->setWireframeLineWidth(highlight_linewidth);
    m_scribeEffectHighlighted->addChild(m_sceneHookHighlighted);
    m_sceneHook->addChild(m_scribeEffectHighlighted);

    //install scribe outlines if m_style!=0 (raw):
    if (m_style!=0) {
      m_scribeEffectNormal->setWireframeColor(m_style==1 ? scribe_color1 : scribe_color2);
      //m_scribeEffectNormal->setWireframeLineWidth(...);
      m_scribeEffectNormal->addChild(m_sceneHookNormal);
      m_sceneHook->addChild(m_scribeEffectNormal);
    } else {
      m_sceneHook->addChild(m_sceneHookNormal);
    }
  } else if (m_style==3) {
    //install cartoon
    m_cartoonEffectHighlighted->setOutlineColor(highlight_color);
    m_cartoonEffectHighlighted->setOutlineLineWidth(highlight_linewidth);
    m_cartoonEffectNormal->setOutlineColor(cartoon_color1);

    m_cartoonEffectHighlighted->addChild(m_sceneHookHighlighted);
    m_sceneHook->addChild(m_cartoonEffectHighlighted);
    m_cartoonEffectNormal->addChild(m_sceneHookNormal);
    m_sceneHook->addChild(m_cartoonEffectNormal);
  // } else if (m_style==4) {

  //   //install outline
  //   m_outlineEffectHighlighted->setColor(highlight_color);
  //   m_outlineEffectHighlighted->setWidth(highlight_linewidth);
  //   m_outlineEffectNormal->setColor(outline_color1);
  //   m_outlineEffectNormal->setWidth(10.);

  //   // m_outlineEffectHighlighted->setColor(osg::Vec4(1,1,0,1));
  //   m_outlineEffectHighlighted->setWidth(80);
  //   // m_outlineEffectNormal->setColor(osg::Vec4(1,1,0,1));
  //   m_outlineEffectNormal->setWidth(80);

  //   m_outlineEffectHighlighted->addChild(m_sceneHookHighlighted);
  //   m_sceneHook->addChild(m_outlineEffectHighlighted);
  //   m_outlineEffectNormal->addChild(m_sceneHookNormal);
  //   m_sceneHook->addChild(m_outlineEffectNormal);

  } else {
    assert(false);//should not happen
  }


  m_viewer->safeStartThreading();

}

//we assume that no one else mess around with HepPolyhedron::SetNumberOfRotationSteps(..)
unsigned G4OSG::VHCommon::getNPhiRendered() const
{
  return static_cast<unsigned>(HepPolyhedron::GetNumberOfRotationSteps());
}

void G4OSG::VHCommon::setNPhiRendered(unsigned n)
{
  if (n==getNPhiRendered())
    return;
  HepPolyhedron::SetNumberOfRotationSteps(static_cast<int>(n));
  m_vhWorld->updateRotationalShapesRecursively();
}

void G4OSG::VHCommon::clearAllSelectedHandles()
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

void G4OSG::VHCommon::toggleSelectHandle(VolHandle*vh,bool multiselect)
{
  bool store = m_blockSelectionChangeCallbacks;
  m_blockSelectionChangeCallbacks=true;

  auto it = m_selectedHandles.find(vh);
  if (it==m_selectedHandles.end())
    selectHandle(vh,multiselect);
  else {
    if (m_selectedHandles.size()>=2) {
      clearAllSelectedHandles();
      selectHandle(vh,multiselect);
    } else {
      deselectHandle(vh,multiselect);
    }
  }
  m_blockSelectionChangeCallbacks=store;
  selectionMightHaveChanged();
}

bool G4OSG::VHCommon::handleIsSelected(const VolHandle*vh) const
{
  return m_selectedHandles.find(const_cast<VolHandle*>(vh))!=m_selectedHandles.end();
}

void G4OSG::VHCommon::selectHandle(VolHandle*vh,bool multiselect)
{
  bool store = m_blockSelectionChangeCallbacks;
  m_blockSelectionChangeCallbacks=true;

  if (!multiselect)
    clearAllSelectedHandles();

  m_selectedHandles.insert(vh);
  vh->setHighlighted(true);

  m_blockSelectionChangeCallbacks=store;
  selectionMightHaveChanged();
}

void G4OSG::VHCommon::deselectHandle(VolHandle*vh,bool multiselect)
{
  bool store = m_blockSelectionChangeCallbacks;
  m_blockSelectionChangeCallbacks=true;

  if (!multiselect)
    clearAllSelectedHandles();

  m_selectedHandles.erase(vh);
  vh->setHighlighted(false);

  m_blockSelectionChangeCallbacks=store;
  selectionMightHaveChanged();
}

//Fixme, temp includes:
#include "G4Material.hh"
#include "G4VSolid.hh"

void G4OSG::VHCommon::selectionMightHaveChanged()
{
  if (m_blockSelectionChangeCallbacks)
    return;
  if (m_selectedHandlesLastCB==m_selectedHandles)
    return;
  m_selectedHandlesLastCB=m_selectedHandles;

  HUD& hud = m_viewer->hud();
  if (m_selectedHandles.empty()) {
    //clear extra info
    hud.clearText(HUD::BOX_CENTER);
  } else {
    assert(m_selectedHandles.size()==1);
    auto vh = *(m_selectedHandles.begin());
    //set extra info
    std::stringstream sss;

    sss <<"Physvol Name   : \""<< vh->pvNameMaxLength(60)<<"\"\n";
    sss <<"Physvol CopyNo : "<< vh->g4PhysVol()->GetCopyNo()<<"\n";
    sss <<"Logvol Name    : \""<< vh->lvNameMaxLength(60)<<"\"\n";
    sss <<"Solid Name     : \""<< vh->solidNameMaxLength(60)<<"\"\n";
    sss <<"Material Name  : \""<< vh->matNameMaxLength(60)<<"\"\n";
    sss <<"\n";
    sss <<"Mother volume  : ";
    //    std::string mname;
    if (vh->mother()) {
      sss<<"\""<<vh->mother()->pvNameMaxLength(60)<<"\"\n";
      // sss<<"\""<<mname<<"\"  --> L+click to access\n";
    } else {
      sss<<"<none>\n";
    }
    sss <<"#daughter vols : "<< vh->nDaughters()<<"\n";//<<(vh->nDaughters()?"  --> X+click to access":"")<<"\n";
#if 0
    sss <<"\n";
    sss <<"#vols with shared Logvol        : n/a\n";
    sss <<"#vols with shared Logvol+CopyNo : n/a\n";
    sss <<"#vols with shared material      : n/a\n";
    sss <<"#vols with shared shape         : n/a\n";
    sss <<"\n";
    sss <<"Volume : n/a cm3\n";
    sss <<"Weight : n/a kg\n";
    if (m_viewer->hasGriffTracks()) {
      sss <<"\n";
      sss <<"<#trks/evt> in volume : n/a\n";
      sss <<"<eDep/evt> in volume  : n/a keV\n";
    }
#endif
    sss <<"\nHit the key 'E' to get extended info dumped to the terminal!";
    // if (vh->mother()||vh->nDaughters()) {
    //   sss <<"\n\nNavigation tips (see more in CTRL-H):\n";
    //   if (vh->nDaughters())
    //     sss <<"\n   Open to daughters: 'X'+Click or MiddleClick.";
    //   if (vh->mother())
    //     sss <<"\n   Close to mother  : 'L'+Click or SHIFT-MiddleClick.";
    // }
    hud.setText(HUD::BOX_CENTER,sss.str());
  }
}

void G4OSG::VHCommon::requestExtendedInfo()
{

  //const G4ThreeVector& G4VPhysicalVolume::GetTranslation() const
  std::cout<<"\n############################# Extended information dump BEGIN #############################\n";
  auto itE = m_selectedHandles.end();
  for (auto it = m_selectedHandles.begin();it!=itE;++it)
    {
      std::cout<<"-------------------- Transformation with respect to mother volume --------------------\n";
      auto t = (*it)->g4PhysVol()->GetTranslation();
      std::cout << "Translation: ("<<G4OSG::BestUnit(t.x(),"Length")<<", "
                <<G4OSG::BestUnit(t.y(),"Length")<<", "
                <<G4OSG::BestUnit(t.z(),"Length")<<")\n";
      std::cout << "Rotation Matrix:";
      auto rot = (*it)->g4PhysVol()->GetRotation();
      if (rot)
        (*it)->g4PhysVol()->GetRotation()->print(std::cout);
      else
        std::cout<<" <none>";
      if (rot) {
        double delta = rot->getDelta();
        G4ThreeVector axis = rot->getAxis();
        std::cout << "Rotation matrix corresponds to:\n";
        std::cout << "    Rotation of    : "<<delta*180.0/M_PI<<" degrees ("<<delta/M_PI<<"*pi)\n";
        std::cout << "    around the axis: "<<axis<<"\n";

      }
      std::cout<<"\n";
      //      std::cout << (*it)->g4PhysVol()->GetRotation()->print()<<"\n";
      std::cout<<"-------------------- Logical Volume \""<<(*it)->g4LogVol()->GetName()<<"\" --------------------\n";
      std::cout<<"\n### Solid dump:\n\n";
      std::cout<<*((*it)->g4Solid())<<std::endl;
      std::cout<<"-----------------------------------------------------------\n";
      std::stringstream smat;
      smat << (*it)->g4LogVol()->GetMaterial();
      std::string ssmat = smat.str();
      auto i = ssmat.find("  density");
      if (i!=std::string::npos)
        ssmat[i]='\n';
      std::cout<<"\n### Material dump:\n\n";
      std::cout<<ssmat;
      std::cout<<"-----------------------------------------------------------" <<std::endl;
    }
  std::cout<<"\n############################# Extended information dump END   #############################\n";
}

void G4OSG::VHCommon::registerZap(VolHandle*vh)
{
  //zapping a volume deselects it: (fixme: only if selected!!)
  deselectHandle(vh,true);

  m_zappedHandles.push_back(vh);
}

void G4OSG::VHCommon::registerUnzap(VolHandle*vh)
{
  unsigned ss=m_zappedHandles.size();
  m_zappedHandles.remove(vh);//the search here is not a particularly fast operation.
  printf("removed %i zapped handles, now %i left\n",ss-int(m_zappedHandles.size()),(int)m_zappedHandles.size());
}

void G4OSG::VHCommon::unzapPrevious()
{
  if (m_zappedHandles.empty())
    return;
  auto itE=m_zappedHandles.rend();
  for (auto it=m_zappedHandles.rbegin();it!=itE;++it) {
    if ((*it)->isDisplayed()) {
      //if not displayed, it would not be rendered after unzapping - confusing the user
      (*it)->unzap();//wasteful to get registerUnzap called through this
      return;
    }
  }
}

void G4OSG::VHCommon::unzapAll()
{
  while (!m_zappedHandles.empty())
    m_zappedHandles.front()->unzap();
}

void G4OSG::VHCommon::resetGeometry()
{
  //unzap all:
  unzapAll();
  //hide everything:
  m_vhWorld->hide();
  m_vhWorld->hideDaughtersRecursively();
  //normal display style recursively, wireframe for world:
  m_vhWorld->setDisplayStyleRecursively(VolHandle::NORMAL);
  m_vhWorld->setDisplayStyle(VolHandle::WIREFRAME);
  //display world and daughters one level down:
  m_vhWorld->displayDaughters();
  m_vhWorld->display();
}
