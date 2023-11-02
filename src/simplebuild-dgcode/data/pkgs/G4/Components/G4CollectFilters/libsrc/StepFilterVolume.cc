#include "G4CollectFilters/StepFilterVolume.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

StepFilterVolume::StepFilterVolume()
  : G4Interfaces::StepFilterBase("G4CollectFilters/StepFilterVolume"),
    m_lastvol(0),
    m_lastvolresult(true)
{
  addParameterString("volumeList","");
  addParameterBoolean("rejectListedVolumes",false);
}

bool StepFilterVolume::validateParameters()
{
  if (getParameterString("volumeList").empty()) {
    printf("StepFilterVolume ERROR: Please specify one or more volumes!");
    return false;
  }
  return true;
}

void StepFilterVolume::initFilter()
{
  //Volumes:
  std::vector<std::string> vols;
  getParameterString_SplitAsVector("volumeList",vols);
  if (vols.empty()) {
    m_volnames_acceptcount=2;//no check
  } else {
    for (unsigned i=0;i<vols.size();++i) {
      m_volnames.insert(vols.at(i));
    }
    m_volnames_acceptcount = getParameterBoolean("rejectListedVolumes") ? 0 : 1;
  }
}

bool StepFilterVolume::filterStep(const G4Step*step) const
{
  if (m_volnames_acceptcount<2) {
    G4LogicalVolume * lv = step->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume();
    if (m_lastvol!=lv) {
      m_lastvol = lv;
      m_lastvolresult = (m_volnames.count(lv->GetName())==m_volnames_acceptcount);
    }
    if (!m_lastvolresult)
      return false;
  }

  return true;
}
