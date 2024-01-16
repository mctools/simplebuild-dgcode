#include "Core/Python.hh"
#include "G4CustomPyGenBase.hh"
#include "G4GunWrapper.hh"

namespace G4CustomPyGen {
  void gun_set_type_v1(G4GunWrapper*g,const char*n) { g->set_type(n); }
  void gun_set_type_v2(G4GunWrapper*g,int pdg) { g->set_type(pdg); }
}

PYTHON_MODULE( mod )
{
  pyextra::pyimport("G4Interfaces");
  py::class_<G4CustomPyGen::GenBaseCpp,G4Interfaces::ParticleGenBase>( mod, "_GenBaseCpp" )
    .def(py::init<>())
    .def("_regpyfct_validatePars",&G4CustomPyGen::GenBaseCpp::regpyfct_validatePars)
    .def("_regpyfct_initGen",&G4CustomPyGen::GenBaseCpp::regpyfct_initGen)
    .def("_regpyfct_genEvt",&G4CustomPyGen::GenBaseCpp::regpyfct_genEvt)
    .def("_py_set_unlimited",&G4CustomPyGen::GenBaseCpp::py_set_unlimited)
    .def("signalEndOfEvents",&G4CustomPyGen::GenBaseCpp::py_signalEndOfEvents)
    ;

  py::class_<G4CustomPyGen::G4GunWrapper, std::shared_ptr<G4CustomPyGen::G4GunWrapper> >( mod, "G4PyGun" )
    .def("set_type",&G4CustomPyGen::gun_set_type_v1)
    .def("set_type",&G4CustomPyGen::gun_set_type_v2)
    .def("set_energy",&G4CustomPyGen::G4GunWrapper::set_energy)
    .def("set_wavelength",&G4CustomPyGen::G4GunWrapper::set_wavelength)
    .def("set_wavelength_angstrom",&G4CustomPyGen::G4GunWrapper::set_wavelength_angstrom)
    .def("set_position",&G4CustomPyGen::G4GunWrapper::set_position)
    .def("set_direction",&G4CustomPyGen::G4GunWrapper::set_direction)
    .def("set_random_direction",&G4CustomPyGen::G4GunWrapper::set_random_direction)
    .def("set_momentum",&G4CustomPyGen::G4GunWrapper::set_momentum)
    .def("set_weight",&G4CustomPyGen::G4GunWrapper::set_weight)
    .def("set_time",&G4CustomPyGen::G4GunWrapper::set_time)
    .def("fire",&G4CustomPyGen::G4GunWrapper::fire)
    .def("allow_empty_events",&G4CustomPyGen::G4GunWrapper::allow_empty_events)
    ;

}
