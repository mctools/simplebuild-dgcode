#include "Core/Python.hh"
#include "G4Interfaces/GeoBase.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Interfaces/StepFilterBase.hh"
#include "G4Interfaces/PhysListProviderBase.hh"
#include "G4Interfaces/FrameworkGlobals.hh"
#include "G4Utils/GenUtils.hh"

namespace{
  namespace G4Interfaces_py {
    void GeoBase_dump_0args(G4Interfaces::GeoBase* g) { g->dump(); }
    void ParticleGenBase_dump_0args(G4Interfaces::ParticleGenBase* g) { g->dump(); }
    void StepFilterBase_dump_0args(G4Interfaces::StepFilterBase* g) { g->dump(); }

    //PB struct to expose protected ParticleGenBase methods to python (solving overloading at the same time):
    struct PGB : public G4Interfaces::ParticleGenBase {
      void setName_v1(const char* name) { setName(name); }
      double rand_v1() { return rand(); }
      double rand_v2(double a, double b) { return rand(a,b); }
      double randgauss_v1(double a, double b) { return randGauss(a,b); }
      double randgauss_v2(double a) { return randGauss(a); }
      double randgauss_v3() { return randGauss(); }
      double randexp_v1(double l) { return randExponential(l); }
      double randexp_v2() { return randExponential(); }
      long randpoisson(double m) { return randPoisson(m); }
      long randint_v1(long n) { return randInt(n); }
      long randint_v2(long a, long b) { return randInt(a,b); }
    };

    void pgb_setName(G4Interfaces::ParticleGenBase*p, const char*n) { static_cast<PGB*>(p)->setName_v1(n); }
    double pgb_rand_v1(G4Interfaces::ParticleGenBase*p) { return static_cast<PGB*>(p)->rand_v1(); }
    double pgb_rand_v2(G4Interfaces::ParticleGenBase*p, double a, double b) { return static_cast<PGB*>(p)->rand_v2(a,b); }
    double pgb_randgauss_v1(G4Interfaces::ParticleGenBase*p, double a, double b) { return static_cast<PGB*>(p)->randgauss_v1(a,b); }
    double pgb_randgauss_v2(G4Interfaces::ParticleGenBase*p, double a) { return static_cast<PGB*>(p)->randgauss_v2(a); }
    double pgb_randgauss_v3(G4Interfaces::ParticleGenBase*p) { return static_cast<PGB*>(p)->randgauss_v3(); }
    double pgb_randexp_v1(G4Interfaces::ParticleGenBase*p, double l) { return static_cast<PGB*>(p)->randexp_v1(l); }
    double pgb_randexp_v2(G4Interfaces::ParticleGenBase*p) { return static_cast<PGB*>(p)->randexp_v2(); }
    long pgb_randpoisson(G4Interfaces::ParticleGenBase*p,double m) { return static_cast<PGB*>(p)->randpoisson(m); }
    long pgb_randint_v1(G4Interfaces::ParticleGenBase*p,long n) { return static_cast<PGB*>(p)->randint_v1(n); }
    long pgb_randint_v2(G4Interfaces::ParticleGenBase*p,long a, long b) { return static_cast<PGB*>(p)->randint_v2(a,b); }

    py::object pgb_randisotropic(G4Interfaces::ParticleGenBase*) {
      G4ThreeVector isodir = G4Utils::randIsotropicDirection();
      return py::make_tuple(isodir.x(),isodir.y(),isodir.z());
    }

  }
}
//todo: expose some of the FrameworkGlobals here??

PYTHON_MODULE( mod )
{
  pyextra::pyimport("Utils.ParametersBase");

  py::class_<G4Interfaces::PhysListProviderBase>(mod, "PhysListProviderBase");

  py::class_<G4Interfaces::GeoBase,Utils::ParametersBase>(mod,"GeoBase")
    .def("getName",&G4Interfaces::GeoBase::getName)
    .def("dump",&G4Interfaces::GeoBase::dump)
    .def("dump",&G4Interfaces_py::GeoBase_dump_0args)
    ;

  //We are not exposing the base G4VUserDetectorConstruction since we have not
  //defined it on the python side (and probably have no need to):

  py::class_<G4Interfaces::GeoConstructBase, G4Interfaces::GeoBase>(mod,"GeoConstructBase");
  py::class_<G4Interfaces::PreGenCallBack, std::shared_ptr<G4Interfaces::PreGenCallBack>>(mod,"PreGenCallBack");
  py::class_<G4Interfaces::PostGenCallBack, std::shared_ptr<G4Interfaces::PostGenCallBack>>(mod, "PostGenCallBack");

  py::class_<G4Interfaces::ParticleGenBase,Utils::ParametersBase>(mod,"ParticleGenBase")
    .def("getName",&G4Interfaces::ParticleGenBase::getName)
    .def("dump",&G4Interfaces::ParticleGenBase::dump)
    .def("dump",&G4Interfaces_py::ParticleGenBase_dump_0args)
    .def("setName",&G4Interfaces_py::pgb_setName)
    .def("rand",&G4Interfaces_py::pgb_rand_v1)
    .def("rand",&G4Interfaces_py::pgb_rand_v2)
    .def("randGauss",&G4Interfaces_py::pgb_randgauss_v1)
    .def("randGauss",&G4Interfaces_py::pgb_randgauss_v2)
    .def("randGauss",&G4Interfaces_py::pgb_randgauss_v3)
    .def("randExponential",&G4Interfaces_py::pgb_randexp_v1)
    .def("randExponential",&G4Interfaces_py::pgb_randexp_v2)
    .def("randPoisson",&G4Interfaces_py::pgb_randpoisson)
    .def("randInt",&G4Interfaces_py::pgb_randint_v1)
    .def("randInt",&G4Interfaces_py::pgb_randint_v2)
    .def("randIsotropic",&G4Interfaces_py::pgb_randisotropic)
    .def("unlimited",&G4Interfaces::ParticleGenBase::unlimited)
    ;

  py::class_<G4Interfaces::StepFilterBase,Utils::ParametersBase>(mod,"StepFilterBase")
    .def("getName",&G4Interfaces::StepFilterBase::getName)
    .def("dump",&G4Interfaces::StepFilterBase::dump)
    .def("dump",&G4Interfaces_py::StepFilterBase_dump_0args)
    ;

  mod.def("isForked",&FrameworkGlobals::isForked);
  mod.def("isParent",&FrameworkGlobals::isParent);
  mod.def("isChild",&FrameworkGlobals::isChild);
  mod.def("mpID",&FrameworkGlobals::mpID);
  mod.def("nProcs",&FrameworkGlobals::nProcs);

}
