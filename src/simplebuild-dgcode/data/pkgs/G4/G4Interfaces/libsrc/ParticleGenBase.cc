#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Interfaces/FrameworkGlobals.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "Randomize.hh"
#include "CLHEP/Random/RandPoisson.h"
#include "G4RunManager.hh"
#include "G4Utils/Flush.hh"
#include <stdexcept>

class ParticleGenBaseAction : public G4VUserPrimaryGeneratorAction
{
public:
  ParticleGenBaseAction(G4Interfaces::ParticleGenBase * g) : m_gen(g), m_first(true) {}
  virtual ~ParticleGenBaseAction(){}
  virtual void GeneratePrimaries(G4Event* evt)
  {
    //Invoke init method the first time around (before the pre-gen callbacks in
    //case the pre-gen callback is used to handle the random seed of an event):
    if (m_first) {
      m_gen->init();
      m_first=false;
    }

    //Fire pre-generation callbacks:
    {
      auto cbIt = m_gen->m_pregencallbacks.begin();
      auto cbItE = m_gen->m_pregencallbacks.end();
      for (;cbIt!=cbItE;++cbIt)
        (*cbIt)->preGen();
    }

    //Actual event generation:
    m_gen->gen(evt);

    //Fire post-generation callbacks:
    {
      auto cbIt = m_gen->m_postgencallbacks.begin();
      auto cbItE = m_gen->m_postgencallbacks.end();
      for (;cbIt!=cbItE;++cbIt)
        (*cbIt)->postGen(evt);
    }
  }
private:
  G4Interfaces::ParticleGenBase * m_gen;
  bool m_first;
};

G4VUserPrimaryGeneratorAction * G4Interfaces::ParticleGenBase::getAction()
{
  if (!m_action)
    m_action = new ParticleGenBaseAction(this);
  return m_action;
}

void G4Interfaces::ParticleGenBase::dump(const char * prefix) const {
  G4Utils::flush();
  printf("%sParticleGenerator[%s]:\n",prefix,m_name.c_str());
  std::string p = prefix;
  p+="  ";
  Utils::ParametersBase::dump(p.c_str());
}

double G4Interfaces::ParticleGenBase::rand()
{
  return CLHEP::HepRandom::getTheEngine()->flat();
}

double G4Interfaces::ParticleGenBase::rand(double a, double b)
{
  return G4RandFlat::shoot(a,b);
}

long G4Interfaces::ParticleGenBase::randInt(long n)
{
  return G4RandFlat::shootInt(n);
}

long G4Interfaces::ParticleGenBase::randInt(long a, long b)
{
  return G4RandFlat::shootInt(a,b);
}

double G4Interfaces::ParticleGenBase::randGauss(double sigma, double mu )
{
  return G4RandGauss::shoot(mu,sigma);
}

double G4Interfaces::ParticleGenBase::randExponential(double lambda)
{
  return G4RandExponential::shoot(CLHEP::HepRandom::getTheEngine(),lambda);
}

long G4Interfaces::ParticleGenBase::randPoisson(double mu)
{
  //return CLHEP::RandPoissonQ::shoot(mu);
  return CLHEP::RandPoisson::shoot(CLHEP::HepRandom::getTheEngine(),mu);
}

void G4Interfaces::ParticleGenBase::installPreGenCallBack( std::shared_ptr<PreGenCallBack> cb )
{
  m_pregencallbacks.push_back(std::move(cb));
}

void G4Interfaces::ParticleGenBase::uninstallPreGenCallBack( std::shared_ptr<PreGenCallBack> pg)
{
  unsigned itarget = m_pregencallbacks.size();
  for (unsigned i=0;i<m_pregencallbacks.size();++i) {
    if (m_pregencallbacks[i]==pg) {
      itarget=i;
      break;
    }
  }
  if ( itarget == m_pregencallbacks.size() )
    throw std::runtime_error("ParticleGenBase::uninstallPreGenCallBack callback not previously installed!");
  if (itarget+1<m_pregencallbacks.size()) {
    for (unsigned i=itarget;i+1<m_pregencallbacks.size();++i)
      m_pregencallbacks.at(i) = std::move(m_pregencallbacks.at(i+1));
  }
  m_pregencallbacks.resize(m_pregencallbacks.size()-1);
}


void G4Interfaces::ParticleGenBase::installPostGenCallBack(std::shared_ptr<PostGenCallBack> cb)
{
  m_postgencallbacks.push_back(std::move(cb));
}

void G4Interfaces::ParticleGenBase::uninstallPostGenCallBack(std::shared_ptr<PostGenCallBack> pg)
{
  unsigned itarget = m_postgencallbacks.size();
  for (unsigned i=0;i<m_postgencallbacks.size();++i) {
    if (m_postgencallbacks[i]==pg) {
      itarget=i;
      break;
    }
  }
  if ( itarget == m_postgencallbacks.size() )
    throw std::runtime_error("ParticleGenBase::uninstallPostGenCallBack callback not previously installed!");
  if (itarget+1<m_postgencallbacks.size()) {
    for (unsigned i=itarget;i+1<m_postgencallbacks.size();++i)
      m_postgencallbacks.at(i) = std::move(m_postgencallbacks.at(i+1));
  }
  m_postgencallbacks.resize(m_postgencallbacks.size()-1);
}

G4Interfaces::ParticleGenBase::ParticleGenBase(const char* name)
  : m_name(name),
    m_action(0),
    m_signalledEOE(false)
{
}

G4Interfaces::ParticleGenBase::~ParticleGenBase()
{
}

void G4Interfaces::ParticleGenBase::signalEndOfEvents(bool including_current_event)
{
  G4Utils::flush();
  if (unlimited())
    throw std::runtime_error("particle generator implementation error : signals end of events despite declaring itself as unlimited");
  if (m_signalledEOE)
    throw std::runtime_error("particle generator implementation error : signals end of events more than once");
  m_signalledEOE = true;
  auto rm = G4RunManager::GetRunManager();
  if (including_current_event) {
    printf("%sWARNING - Insufficient data - aborting current event.\n",FrameworkGlobals::printPrefix());
    rm->AbortRun(false);
  } else {
    rm->AbortRun(true);
  }
  G4Utils::flush();
}

bool G4Interfaces::ParticleGenBase::reachedLimit() const
{
  return m_signalledEOE;
}
