#include "GriffDataRead/DumpObj.hh"
#include "GriffDataRead/Track.hh"
#include "GriffDataRead/Segment.hh"
#include "GriffDataRead/Step.hh"
#include "GriffDataRead/Material.hh"
#include "GriffDataRead/Element.hh"
#include "GriffDataRead/Isotope.hh"
#include "Units/Units.hh"
#include <iostream>

void GriffDataRead::dump(const Track*t, bool dumpPDGInfo)
{
  assert(t);
  const double ue(Units::keV);
  const double ut(Units::ms);
  printf("[track(units:ms,keV):");
  printf(" id=%i",t->trackID());
  printf(" parent=%i",t->parentID());
  for (unsigned i=0;i<t->nDaughters();++i)
    printf(" daug%i=%i",i,t->getDaughterID(i));
  printf(" nsegm=%i",t->nSegments());
  printf(" w=%f",t->weight());
  printf(" primary=%i",t->isPrimary()?1:0);
  printf(" secondary=%i",t->isSecondary()?1:0);
  printf(" t0[ms]=%f",t->startTime()/ut);
  printf(" ekin0[keV]=%f",t->startEKin()/ue);
  printf(" pdg=%i",t->pdgCode());
  printf(" proc=%s",t->creatorProcessCStr());
  if (dumpPDGInfo) {
    printf(" name=%s",t->pdgNameCStr());
    printf(" type=%s",t->pdgTypeCStr());
    printf(" subtype=%s",t->pdgSubTypeCStr());
    printf(" mass=%f",t->mass()/ue);
    printf(" width=%f",t->width()/ue);
    printf(" charge=%f",t->charge());//unitless
    printf(" lifeTime=%f",t->lifeTime()/ut);
    printf(" atomicNumber=%i",t->atomicNumber());
    printf(" atomicMass=%i",t->atomicMass());
    constexpr double nuclearMagneton = (Constants::h_Planck/(4*Constants::pi))/(Constants::proton_mass_c2 /Constants::c_squared);
    printf(" magneticMoment[muN]=%f",t->magneticMoment()/nuclearMagneton);
    printf(" spin=%f",t->spin());
    printf(" stable=%i",t->stable()?1:0);
    printf(" shortLived=%i",t->shortLived()?1:0);
  }
}

void GriffDataRead::dump(const Segment*s)
{
  assert(s);
  const double ue(Units::keV);
  const double ut(Units::ms);
  printf("[segment(units:ms,keV):");
  printf(" t0=%f",s->startTime()/ut);
  printf(" t1=%f",s->endTime()/ut);
  printf(" ekin0=%f",s->startEKin()/ue);
  printf(" ekin1=%f",s->endEKin()/ue);
  printf(" edep=%f",s->eDep()/ue);
  printf(" edepnonion=%f",s->eDepNonIonising()/ue);
  printf(" edge0=%i",s->startAtVolumeBoundary()?1:0);
  printf(" edge1=%i",s->endAtVolumeBoundary()?1:0);
  for(unsigned i=0;i<s->volumeDepthStored();++i) {
    printf(" v%i=%s#%s#%s#%i",i,s->volumeNameCStr(i),s->physicalVolumeNameCStr(i),s->material(i)->getNameCStr(),s->volumeCopyNumber(i));
  }
  printf("]");
}

void GriffDataRead::dump(const Step*s)
{
  assert(s);
  const double ue(Units::keV);
  const double ut(Units::ms);
  const double ul(Units::mm);
  printf("[step(units:ms,mm,keV):");
  printf(" edep=%f",s->eDep()/ue);
  printf(" edepnonion=%f",s->eDepNonIonising()/ue);
  printf(" stepLength=%f",s->stepLength()/ul);
  printf(" stepStatus=%s",s->stepStatusCStr());
  printf(" t0=%f",s->preTime()/ut);
  printf(" t1=%f",s->postTime()/ut);
  printf(" ekin0=%f",s->preEKin()/ue);
  printf(" ekin1=%f",s->postEKin()/ue);
  printf(" x0=%f",s->preGlobalX()/ul);
  printf(" y0=%f",s->preGlobalY()/ul);
  printf(" z0=%f",s->preGlobalZ()/ul);
  printf(" x1=%f",s->postGlobalX()/ul);
  printf(" y1=%f",s->postGlobalY()/ul);
  printf(" z1=%f",s->postGlobalZ()/ul);
  printf(" lx0=%f",s->preLocalX()/ul);
  printf(" ly0=%f",s->preLocalY()/ul);
  printf(" lz0=%f",s->preLocalZ()/ul);
  printf(" lx1=%f",s->postLocalX()/ul);
  printf(" ly1=%f",s->postLocalY()/ul);
  printf(" lz1=%f",s->postLocalZ()/ul);
  printf(" edge0=%i",(s->preAtVolEdge()?1:0));
  printf(" edge1=%i",(s->postAtVolEdge()?1:0));
  printf(" px0=%f",s->preMomentumX()/ue);
  printf(" py0=%f",s->preMomentumY()/ue);
  printf(" pz0=%f",s->preMomentumZ()/ue);
  printf(" px1=%f",s->postMomentumX()/ue);
  printf(" py1=%f",s->postMomentumY()/ue);
  printf(" pz1=%f",s->postMomentumZ()/ue);
  printf(" proc0=%s",s->preProcessDefinedStepCStr());
  printf(" proc1=%s",s->postProcessDefinedStepCStr());
  printf("]");
}


void GriffDataRead::dump(const Material* mat)
{
  assert(mat);
  printf("[material:");
  printf(" name=%s",mat->getNameCStr());
  printf(" density=%fg/cm3",mat->density()/(Units::g/Units::cm3));
  printf(" temp=%fK",mat->temperature()/Units::kelvin);
  printf(" pressure=%fbar",mat->pressure()/Units::bar);
  printf(" radlen=%fmm",mat->radiationLength()/Units::mm);
  printf(" nuclintlen=%fmm",mat->nuclearInteractionLength()/Units::mm);
  printf(" Imean=%feV",(mat->hasMeanExitationEnergy()?mat->meanExitationEnergy()/Units::eV:-1));
  printf(" state=%s",mat->stateCStr());
  unsigned nelem(mat->numberElements());
  for(unsigned ielem=0;ielem<nelem;++ielem)
    printf(" elem%i[%f%%]=%s",ielem,100.0*mat->elementFraction(ielem),mat->getElement(ielem)->getNameCStr());
  printf("]");
}

void GriffDataRead::dump(const Element*elem)
{
  printf("[element:");
  printf(" name=%s",elem->getNameCStr());
  printf(" symbol=%s",elem->getSymbolCStr());
  printf(" Z=%f",elem->Z());
  printf(" N=%f",elem->N());
  printf(" A=%fg/mole",elem->A()/(Units::g/Units::mole));
  printf(" naturalabundance=%s",elem->naturalAbundances()?"yes":"no");

  unsigned niso(elem->numberIsotopes());
  for (unsigned iiso=0;iiso<niso;++iiso)
    printf(" iso%i[%f%%]=%s(%i,%i)",iiso,
           100.0*elem->isotopeRelativeAbundance(iiso),
           elem->getIsotope(iiso)->getNameCStr(),
           elem->getIsotope(iiso)->Z(),
           elem->getIsotope(iiso)->N());
  printf("]");
}

void GriffDataRead::dump(const Isotope*iso)
{
  printf("[isotope:");
  printf(" name=%s",iso->getNameCStr());
  printf(" Z=%i",iso->Z());
  printf(" N=%i",iso->N());
  printf(" A=%fg/mole",iso->A()/(Units::g/Units::mole));
  printf(" m=%i",iso->m());
  printf("]");
}
