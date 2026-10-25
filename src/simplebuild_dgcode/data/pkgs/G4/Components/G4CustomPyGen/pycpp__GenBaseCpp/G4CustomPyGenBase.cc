#include "G4CustomPyGenBase.hh"

//NB: The methods below which use Python objects acquire the GIL, since they can
//be called from code which is itself called from Python without the GIL (in
//particular from G4Launcher, which is an sbldpy based module).

G4CustomPyGen::GenBaseCpp::GenBaseCpp()
  : ParticleGenBase("_tmpname_"),//Name will be changed on python side
    m_gunwrapper(0),
    m_unlimited(true),
    m_currentEventAborted(false)
{
}

G4CustomPyGen::GenBaseCpp::~GenBaseCpp()
{
  py::gil_scoped_acquire gil;
  //release refs by assigning to None:
  m_gunwrapper_pyobj = py::object();
  m_pyfct_validatePars = py::object();
  m_pyfct_initGen = py::object();
  m_pyfct_genEvt = py::object();
  delete m_gunwrapper;//todo: test what happens if user keeps ref
}

void G4CustomPyGen::GenBaseCpp::init()
{
  py::gil_scoped_acquire gil;
  m_gunwrapper = new G4GunWrapper;
  m_gunwrapper_pyobj = py::cast(m_gunwrapper);
  assert(getNameStr()!="_tmpname_");
  if (m_pyfct_initGen)
    m_pyfct_initGen(m_gunwrapper_pyobj);
}

void G4CustomPyGen::GenBaseCpp::gen( G4Event * evt )
{
  py::gil_scoped_acquire gil;
  assert(m_pyfct_genEvt);
  m_gunwrapper->setEvent(evt);
  m_pyfct_genEvt(m_gunwrapper_pyobj);
  //Fire once unless user did so (or allowed empty events):
  if (!m_currentEventAborted) {
    m_gunwrapper->fireIfNeeded();
  } else {
    if (m_gunwrapper->hasFired())
      throw std::runtime_error("implementation error: Custom python generator aborted current event *after* it fired!\n");
  }
}

bool G4CustomPyGen::GenBaseCpp::validateParameters()
{
  py::gil_scoped_acquire gil;
  if (m_pyfct_validatePars) {
    py::object res = m_pyfct_validatePars();
    return res.cast<bool>();
  }
  return true;
}

void G4CustomPyGen::GenBaseCpp::regpyfct_validatePars(py::object o)
{
  assert(o);
  m_pyfct_validatePars=o;
}

void G4CustomPyGen::GenBaseCpp::regpyfct_initGen(py::object o)
{
  assert(o);
  m_pyfct_initGen=o;
}

void G4CustomPyGen::GenBaseCpp::regpyfct_genEvt(py::object o)
{
  assert(o);
  m_pyfct_genEvt=o;
}
