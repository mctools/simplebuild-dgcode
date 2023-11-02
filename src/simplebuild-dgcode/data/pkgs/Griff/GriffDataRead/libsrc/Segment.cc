#include <cassert>
#include "GriffDataRead/Segment.hh"
#include "GriffDataRead/Track.hh"
#include "GriffDataRead/GriffDataReader.hh"
#include "GriffDataRead/DumpObj.hh"

const GriffDataRead::Touchable& GriffDataRead::Segment::getTouchable() const
{
  static_assert(sizeof(double)==8&&sizeof(EvtFile::index_type)==4&&sizeof(float)==4);
  return m_trk->m_dr->m_dbTouchables.getEntry(touchableIndex());
}

double GriffDataRead::Segment::segmentLength() const
{
  setupSteps();
  double sl(0.0);
  const Step * sE(stepEnd());
  for (const Step * s=stepBegin();s!=sE;++s) {
    sl+=s->stepLength();
  }
  return sl;
}

void GriffDataRead::Segment::actualSetupSteps() const
{
  assert(!m_stepsBegin);
  int32_t rawstep = rawStepIdx();
  if (rawstep<0) {
    //minimal mode
    m_stepsEnd=(const Step*)0x1;
    return;
  }
  GriffDataReader * dr = m_trk->m_dr;
  const char * stepdata = dr->m_fr->getFullData() + rawstep;
  unsigned nsteps_stored = ByteStream::interpret<std::uint32_t>(stepdata + 4);
  assert(nsteps_stored>=1);
  stepdata+=GriffFormat::Format::SIZE_STEPHEADER;
  static_assert(GriffFormat::Format::SIZE_STEPHEADER==8);
  //Get memory big enough to store nsteps_stored Step objects:
  if (nsteps_stored<=GriffDataReader::MEMPOOL_STEPS_NSTEPS) {
    m_stepsBegin = (const Step*) dr->m_mempool_steps.acquire();
  } else {
    m_stepsBegin = (const Step*) new char[sizeof(Step)*nsteps_stored];
    dr->m_mempool_dynamic.push_back((char*)m_stepsBegin);
  }
  assert(m_stepsBegin);
  m_stepsEnd = m_stepsBegin + nsteps_stored;
  const Step* step = m_stepsBegin;
  for (;step!=m_stepsEnd;++step) {
    assert(stepdata);
    const_cast<Step*>(step)->set(this,stepdata);
    assert(step->m_data);
    stepdata+=(GriffFormat::Format::SIZE_STEPPREPOSTPART+GriffFormat::Format::SIZE_STEPOTHERPART);
    static_assert(GriffFormat::Format::SIZE_STEPPREPOSTPART==68);
    static_assert(GriffFormat::Format::SIZE_STEPOTHERPART==16);
  }
  assert(m_stepsBegin->m_data);
  assert((m_stepsEnd-1)->m_data);
  assert((int)nStepsStored()==m_stepsEnd-m_stepsBegin);
}
