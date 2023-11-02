#include "DBMaterialEntry.hh"
#include "DBElementEntry.hh"
#include "DCMgr.hh"

void G4DataCollectInternals::DBMaterialEntry::write(EvtFile::FileWriter&fw)
{
  fw.writeDataDBSection((int32_t)0);//version
  fw.writeDataDBSection(m_mgr->dbMaterialNames.getIndex(m_mat->GetName()));//name
  fw.writeDataDBSection(double(m_mat->GetDensity()));//density
  fw.writeDataDBSection(double(m_mat->GetTemperature()));//temperature
  fw.writeDataDBSection(double(m_mat->GetPressure()));//pressure
  fw.writeDataDBSection(double(m_mat->GetRadlen()));//radiation length
  fw.writeDataDBSection(double(m_mat->GetNuclearInterLength()));//nuclear interaction length
  auto ion = m_mat->GetIonisation();
  fw.writeDataDBSection(double(ion ? ion->GetMeanExcitationEnergy() : -1.0 ));//mean excitation energy
  int32_t state = (int32_t)m_mat->GetState();
  static_assert(kStateUndefined==0);
  static_assert(kStateSolid==1);
  static_assert(kStateLiquid==2);
  static_assert(kStateGas==3);
  assert(state>=0&&state<=3);
  fw.writeDataDBSection(state);//solid/liquid/gas
  std::uint32_t nelem = m_mat->GetNumberOfElements();
  fw.writeDataDBSection(nelem);//number of elements

  for (unsigned i = 0; i<nelem; ++i) {
    const G4Element * elem = m_mat->GetElement(i);
    double fraction = m_mat->GetFractionVector()[i];
    DBElementEntry * elemEntry = new DBElementEntry(elem,m_mgr);
    elemEntry->ref();
    EvtFile::index_type elemIdx = m_mgr->dbElements.getIndex(elemEntry);
    elemEntry->unref();
    fw.writeDataDBSection(fraction);//fraction i
    fw.writeDataDBSection(elemIdx);//element index i
  }
}
