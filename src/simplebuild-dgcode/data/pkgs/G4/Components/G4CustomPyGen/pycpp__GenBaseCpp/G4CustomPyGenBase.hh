#ifndef G4CustomPyGen_pycpp_G4CustomPyGenBase_hh
#define G4CustomPyGen_pycpp_G4CustomPyGenBase_hh

#include "Core/Python.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4GunWrapper.hh"

namespace G4CustomPyGen {

  //NB: Hidden, since py::object is hidden (OK, since this is a private header
  //intended for a single shared library).
  class __attribute__ ((visibility ("hidden"))) GenBaseCpp : public G4Interfaces::ParticleGenBase {
  public:
    GenBaseCpp();
    virtual ~GenBaseCpp();

    //The ParticleGenBase interface:
    void init();
    void gen(G4Event*);

    //Used by python-side to register the user defined python functions:
    void regpyfct_validatePars(py::object);
    void regpyfct_initGen(py::object);
    void regpyfct_genEvt(py::object);

    virtual bool unlimited() const { return m_unlimited; }

    void py_set_unlimited(bool u) { m_unlimited = u; }

    void py_signalEndOfEvents(bool u) { m_currentEventAborted = u; signalEndOfEvents(u); }

  protected:
    bool validateParameters();
    G4GunWrapper * m_gunwrapper;
    py::object m_gunwrapper_pyobj;
    py::object m_pyfct_validatePars;
    py::object m_pyfct_initGen;
    py::object m_pyfct_genEvt;
    bool m_unlimited;
    bool m_currentEventAborted;
  };
}


#endif
