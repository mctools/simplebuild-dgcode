#include "DCStepData.hh"
#include "G4Interfaces/FrameworkGlobals.hh"
#include <cassert>
#include "G4Step.hh"
#include "G4NavigationHistory.hh"
#include "G4AffineTransform.hh"
#include "G4VProcess.hh"
#include "G4StepStatus.hh"

void G4DataCollectInternals::DCStepData::EndPointData::set(G4StepPoint*p,const G4AffineTransform& topTransform,DCMgr&mgr)
{
  assert(p);
  time = p->GetGlobalTime();
  eKin = p->GetKineticEnergy();
  G4ThreeVector tmp = p->GetPosition();
  globpos[0]=tmp.x();globpos[1]=tmp.y();globpos[2]=tmp.z();
  tmp = topTransform.TransformPoint(tmp);
  locpos[0]=tmp.x();locpos[1]=tmp.y();locpos[2]=tmp.z();
  tmp = p->GetMomentum();
  mom[0]=tmp.x();mom[1]=tmp.y();mom[2]=tmp.z();
  weight = p->GetWeight();
  atVolEdge = (p->GetStepStatus() == fGeomBoundary || p->GetStepStatus() == fWorldBoundary);


  //Get the index for the name of the GetProcessDefinedStep, but avoid repeated lookups of the same name.
  const G4VProcess * proc = p->GetProcessDefinedStep();
  assert(proc!=(const G4VProcess *)0x1);
  static const G4VProcess * lastProc = (const G4VProcess *)0x1;
  static EvtFile::index_type lastProcIdx = 0;
  if (proc!=lastProc) {
    static G4String empty;
    const G4String * name = proc ? &(proc->GetProcessName()) : &empty;
    assert(name);
    lastProcIdx = mgr.dbProcNames.getIndex(*name);
    lastProc = proc;
  }
  processDefiningStepIdx = lastProcIdx;
}

void G4DataCollectInternals::DCStepData::set(const G4Step*step,DCMgr&mgr, int isNewVolOnSameTrack,G4DataCollectInternals::DCStepData* prevStep)
{
#ifdef GRIFF_EXTRA_TESTS
  //should have just been constructed or clear'ed
  assert(!valid());
#endif
  auto preStepPoint = step->GetPreStepPoint();
  auto postStepPoint = step->GetPostStepPoint();
  auto track = step->GetTrack();
  auto touchable = preStepPoint->GetTouchableHandle();

  //ID's:
  trkId = track->GetTrackID();
  parentId = track->GetParentID();
  stepNbr = track->GetCurrentStepNumber();
  assert(stepNbr>=1);
  pdgcode = track->GetDynamicParticle()->GetPDGcode();

  if (!pdgcode) {
    //this could be a special particle such as an opticalphoton.
    pdgcode = mgr.dbPDGCodes.getAdHocPDGCode(track->GetDynamicParticle()->GetParticleDefinition());
    assert(pdgcode);
  }
  //info valid for entire step - edep:
  eDep = step->GetTotalEnergyDeposit();
  eDepNonIonizing = step->GetNonIonizingEnergyDeposit();

  //info valid for entire step - status:
  int step_status_int = postStepPoint->GetStepStatus();//NB: status from poststep means this step, prestep gives status for previous step
  assert(step_status_int>=0&&step_status_int<=7);//fits in one byte
  static_assert((int)fWorldBoundary==0);// Step reached the world boundary
  static_assert((int)fGeomBoundary==1);// Step defined by a geometry boundary
  static_assert((int)fAtRestDoItProc==2);// Step defined by a PreStepDoItVector
  static_assert((int)fAlongStepDoItProc==3);// Step defined by a AlongStepDoItVector
  static_assert((int)fPostStepDoItProc==4);// Step defined by a PostStepDoItVector
  static_assert((int)fUserDefinedLimit==5);// Step defined by the user Step limit in the logical volume
  static_assert((int)fExclusivelyForcedProc==6);// Step defined by an exclusively forced PostStepDoIt process
  static_assert((int)fUndefined==7);// Step not defined yet
  stepStatus = (std::uint32_t)step_status_int;

  //post and pre step info:
  const G4AffineTransform& topTransform = touchable->GetHistory()->GetTopTransform();
  preStep.set(preStepPoint,topTransform,mgr);
  postStep.set(postStepPoint,topTransform,mgr);

  //info valid for entire step - length:
  stepLength = step->GetStepLength();

  if (stepNbr<2) {
    const G4VProcess* creatorproc = track->GetCreatorProcess();
    static G4String empty;
    creatorProcessName = creatorproc ? &(creatorproc->GetProcessName()) : &empty;
  } else {
    creatorProcessName = 0;
  }

  segmentEnd = 0;

  volIdx = EvtFile::INDEX_MAX;//will be replaced by proper value during end of event processing
  if (preStep.atVolEdge||stepNbr<2) {
    bool notNewVolumeNoMatterWhatGeant4Says = (isNewVolOnSameTrack==0);
    if (notNewVolumeNoMatterWhatGeant4Says) {
      std::cout<<FrameworkGlobals::printPrefix()<<"WARNING: Correcting buggy Geant4 AtVolEdge flags."
        " Seed is "<<FrameworkGlobals::currentEvtSeed()<<std::endl;
      if (prevStep&&prevStep->trkId==trkId&&prevStep->stepNbr+1==stepNbr)
        prevStep->postStep.atVolEdge = false;
      preStep.atVolEdge = false;
      touchableEntry = 0;
    } else {
      //We likely passed into (or started in) a new volume:
      touchableEntry = new DBTouchableEntry(&mgr,touchable);//fixme: use memory pool...
      touchableEntry->ref();
#ifdef GRIFF_EXTRA_TESTS
      touchableEntry->extraTests();
#endif
    }
  } else {
    bool newVolumeNoMatterWhatGeant4Says = (stepNbr>=2&&isNewVolOnSameTrack==1);
    if (newVolumeNoMatterWhatGeant4Says) {
      std::cout<<FrameworkGlobals::printPrefix()<<"WARNING: Correcting buggy Geant4 AtVolEdge flags."
        " Seed is "<<FrameworkGlobals::currentEvtSeed()<<std::endl;
      if (prevStep&&prevStep->trkId==trkId&&prevStep->stepNbr+1==stepNbr)
        prevStep->postStep.atVolEdge = true;
      preStep.atVolEdge = true;
      touchableEntry = new DBTouchableEntry(&mgr,touchable);//fixme: use memory pool...
      touchableEntry->ref();
#ifdef GRIFF_EXTRA_TESTS
      touchableEntry->extraTests();
#endif
    } else {
      touchableEntry = 0;
    }
  }
#ifdef GRIFF_EXTRA_TESTS
  assert(valid());
#endif
}
