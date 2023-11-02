#ifndef G4Interfaces_ParticleGenBase_hh
#define G4Interfaces_ParticleGenBase_hh

#include "Utils/ParametersBase.hh"
#include <vector>
#include <utility>
#include <memory>

class G4VUserPrimaryGeneratorAction;
class G4Event;
class ParticleGenBaseAction;

namespace G4Interfaces {

  class PreGenCallBack;
  class PostGenCallBack;

  class ParticleGenBase : public Utils::ParametersBase
  {
  public:
    //Derived classes must:
    //
    //  1) Provide a name with  : ParticleGenBase("some name")
    //  2) Use the ParametersBase addParameterXXX calls in the constructor to
    //     define free parameters which the particle generation will depend on.
    //  3) Implement particle generation in (using getParameterXXX calls):
    //        void init();
    //        void gen(G4Event*);

    ParticleGenBase(const char* name);
    virtual ~ParticleGenBase();
    const char* getName() const { return m_name.c_str(); }
    const std::string& getNameStr() const { return m_name; }
    void dump(const char * prefix="") const;

    virtual void init() {}
    virtual void gen(G4Event*) = 0;

    //The next method must not be called before the physics lists have been
    //initialised (With the G4Launcher framework there is no need to call this
    //yourself):
    G4VUserPrimaryGeneratorAction * getAction();

    //The last method can be used to install callbacks *before* each event
    //generation (this is not possible with G4UserEventAction).
    void installPreGenCallBack(std::shared_ptr<PreGenCallBack>);
    void uninstallPreGenCallBack(std::shared_ptr<PreGenCallBack>);//uninstall
    //And right after (to monitor or modify the event):
    void installPostGenCallBack(std::shared_ptr<PostGenCallBack>);
    void uninstallPostGenCallBack(std::shared_ptr<PostGenCallBack>);//uninstall

    //Override and return false if generator does not have unlimited events
    //available (like for a generator reading particles from an input file). In
    //that case, the generator must invoke signalEndOfEvents (see below) when
    //appropriate:
    virtual bool unlimited() const { return true; }

    //returns true if generator has signalled end of events:
    bool reachedLimit() const;

  protected:
    //convenient access to random numbers:
    double rand();// flat in 0..1
    double rand(double a, double b);//flat in a..b
    double randGauss(double sigma = 1.0, double mu = 0.0 );
    double randExponential(double lambda = 1.0);
    long randPoisson(double mu);
    long randInt(long n);
    long randInt(long a, long b);
    //Rarely, some implementations might need to change the name
    //post-constructor:
    void setName(const char* name) { m_name = name; }
    //Implementations with limited number of events (like those based on
    //particles listed in input files) can signal that they are out of events by
    //calling the following methods during invocation of gen(..). Setting
    //including_current_event to true indicates that even the current event can
    //not be generated.
    void signalEndOfEvents(bool including_current_event);
  private:
    friend class ::ParticleGenBaseAction;
    std::string m_name;
    G4VUserPrimaryGeneratorAction * m_action;
    std::vector<std::shared_ptr<PreGenCallBack>> m_pregencallbacks;
    std::vector<std::shared_ptr<PostGenCallBack>> m_postgencallbacks;
    bool m_signalledEOE;
  };

  class PreGenCallBack : public std::enable_shared_from_this<PreGenCallBack> {
  public:
    virtual void preGen() = 0;
    PreGenCallBack(){}
    virtual ~PreGenCallBack(){}
  };

  class PostGenCallBack : public std::enable_shared_from_this<PostGenCallBack> {
  public:
    virtual void postGen(G4Event*) = 0;
    PostGenCallBack(){}
    virtual ~PostGenCallBack(){}
  };


}

#endif
