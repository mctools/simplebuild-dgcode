#include "G4Random/RandomManager.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include <cassert>
#include "G4Utils/Flush.hh"
#include "CLHEP/Random/Random.h"
#include "G4Interfaces/FrameworkGlobals.hh"
// #include "NCG4RngEngine.hh"

#include "CLHEP/Random/MixMaxRng.h"

namespace {

  void setMixMaxRNG64bitSeed( CLHEP::MixMaxRng& engine, std::uint64_t seed64bit )
  {
    std::uint32_t seed0 = seed64bit && 0xFFFFFFFF;
    std::uint32_t seed1 = ( seed64bit >> 32 ) && 0xFFFFFFFF;
    long seeds_long[2];
    seeds_long[0] = static_cast<long>(seed0);
    seeds_long[1] = static_cast<long>(seed1);
    engine.setSeeds( &seeds_long[0], 2 );
  }

  std::uint64_t getMixMaxRNG64bitvalue( CLHEP::MixMaxRng& engine )
  {
    unsigned int i0( engine );
    unsigned int i1( engine );
    unsigned int i2( engine );
    unsigned int i3( engine );
    std::uint64_t result = i0;
    result <<= 16;
    result |= ( i1 & 0xFFFF );
    result <<= 16;
    result |= ( i2 & 0xFFFF );
    result <<= 16;
    result |= ( i3 & 0xFFFF );
    return result;
  }

}

class RndmSeedCB;
static std::shared_ptr<RndmSeedCB> s_theRndmSeedCB = nullptr;
class RndmSeedCB : public G4Interfaces::PreGenCallBack {
public:

  //Will set the seed at the beginning of each event and print it (occasionally)
  //along with the event number. First event will start with the seed passed in
  //through the variable "firstseed".
  RndmSeedCB(std::uint64_t firstseed,RandomManager::EVTMSGLEVEL l)
    : PreGenCallBack(),
      m_nextseed(firstseed),
      m_evtcount(0),
      m_evtMsgLvl(l)
  {
    printf("%sInstalling xoroshiro128+ random generator (via NCrystal)\n",FrameworkGlobals::printPrefix());
    //m_engine = new NCG4RngEngine;
    m_engine = new CLHEP::MixMaxRng;
    CLHEP::HepRandom::setTheEngine(m_engine);
    setMixMaxRNG64bitSeed( *m_engine, 23487653 );//This seed will be used only
                                                 //during initialisation, so
                                                 //silently having it be the
                                                 //same in all jobs means that
                                                 //the "event seed" will be
                                                 //actually useful to reproduce
                                                 //those events.

    // m_engine->dgcode_set64BitSeed(23487653);//This seed will be used only during
    //                                         //initialisation, so silently having it be the
    //                                         //same in all jobs means that the "event seed"
    //                                         //will be actually useful to reproduce those
    //                                         //events.
  }
  virtual void preGen()
  {
    std::uint64_t seed_to_use;
    if (m_nextseed) {
      //1st event...
      if (FrameworkGlobals::isForked()&&FrameworkGlobals::isChild()) {
        //...in a forked off child.

        //TODO: We should have a separate stream to generate evt seeds. That way
        //we can ensure to get the same distributions irrespective of number of
        //processes (also, we should fix the evt/run numbers).

        const std::uint64_t large_prime = UINT64_C(541234505579);//something larger than 4 billion
                                                            //=> should hopefully avoid
                                                            //clashes with multiple seeds
                                                            //specified by the user
        m_nextseed += large_prime*FrameworkGlobals::mpID();
        assert(m_nextseed);
      }
      seed_to_use = m_nextseed;
      m_nextseed=0;
    } else {
      //Generate a random seed for this event:
      assert(m_engine);
      seed_to_use = getMixMaxRNG64bitvalue( *m_engine );
      //seed_to_use = m_engine->dgcode_genHighQuality64bitUint();
    }
    setMixMaxRNG64bitSeed( *m_engine, seed_to_use );
    //m_engine->dgcode_set64BitSeed(seed_to_use);
    FrameworkGlobals::setCurrentEvtSeed(seed_to_use);

    if (m_evtMsgLvl!=RandomManager::EVTMSG_NEVER) {
      ++m_evtcount;
      if (m_evtMsgLvl==RandomManager::EVTMSG_ALWAYS ||
          (m_evtcount<11
           ||(m_evtcount<101&&m_evtcount%10==0)
           ||(m_evtcount<1001&&m_evtcount%100==0)
           ||(m_evtcount<10001&&m_evtcount%1000==0)
           ||(m_evtcount%10000==0)) )
        {
          G4Utils::flush();
          //don't use printf for 64bit ints as it is less portable:
          std::cout << FrameworkGlobals::printPrefix()
                    <<"Begin simulation of event "<<m_evtcount
                    <<" [seed " << seed_to_use<<"]"
                    <<std::endl;
        }
    }
  }
  virtual ~RndmSeedCB()
  {
    assert(s_theRndmSeedCB.get()==this);
    s_theRndmSeedCB = nullptr;
  }
private:
  std::uint64_t m_nextseed;
  //  NCG4RngEngine * m_engine = nullptr;
  CLHEP::MixMaxRng * m_engine = nullptr;
  unsigned m_evtcount;
  RandomManager::EVTMSGLEVEL m_evtMsgLvl;
};

void RandomManager::init(std::uint64_t seed_of_first_event,EVTMSGLEVEL lvl)
{
  assert(!s_theRndmSeedCB);
  s_theRndmSeedCB = std::make_shared<RndmSeedCB>(seed_of_first_event,lvl);
}

void RandomManager::attach(G4Interfaces::ParticleGenBase* pg)
{
  pg->installPreGenCallBack(s_theRndmSeedCB);
}
///home/tkittel/scr/miniforge3/envs/sb/include/CLHEP/Random/MixMaxRng.h
