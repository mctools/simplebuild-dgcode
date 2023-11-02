#include "Core/Python.hh"
#include "Utils/ParametersBase.hh"

namespace Utils_ParametersBase_py {
  void dump_0args(Utils::ParametersBase*p) { p->dump(); }
  void dumpNoLock_0args(Utils::ParametersBase*p) { p->dumpNoLock(); }

  void tieParameters_v1(Utils::ParametersBase*p,const std::string&n,Utils::ParametersBase*o,const std::string&on) { p->tieParameters(n,o,on); }
  void tieParameters_v2(Utils::ParametersBase*p,const std::string&n1,const std::string&n2) { p->tieParameters(n1,n2); }

  void exposeParameter_2args(Utils::ParametersBase*p,const std::string& on, Utils::ParametersBase*o) { p->exposeParameter(on,o); }
  void exposeParameter_3args(Utils::ParametersBase*p,const std::string& on, Utils::ParametersBase*o, const std::string& nn) { p->exposeParameter(on,o,nn); }

  void exposeParametersv1_1arg(Utils::ParametersBase*p,Utils::ParametersBase*o) { p->exposeParameters(o); }
  void exposeParametersv1_2args(Utils::ParametersBase*p,Utils::ParametersBase*o,const std::string&pref) { p->exposeParameters(o,pref); }
  void exposeParametersv1_3args(Utils::ParametersBase*p,Utils::ParametersBase*o,const std::string&pref,bool toc) { p->exposeParameters(o,pref,toc); }

  //TODO exposeParametersv2 with exclusion list

  py::list getParameterListDouble(Utils::ParametersBase*p)
  {
    std::vector<std::string> v;
    p->getParameterListDouble(v);
    py::list pylist = py::list();
    auto itE=v.end();
    for (auto it=v.begin();it!=itE;++it) {
      pylist.append(*it);
    }
    return pylist;
  }
  py::list getParameterListInt(Utils::ParametersBase*p)
  {
    std::vector<std::string> v;
    p->getParameterListInt(v);
    py::list pylist = py::list();
    auto itE=v.end();
    for (auto it=v.begin();it!=itE;++it) {
      pylist.append(*it);
    }
    return pylist;
  }
  py::list getParameterListBoolean(Utils::ParametersBase*p)
  {
    std::vector<std::string> v;
    p->getParameterListBoolean(v);
    py::list pylist = py::list();
    auto itE=v.end();
    for (auto it=v.begin();it!=itE;++it) {
      pylist.append(*it);
    }
    return pylist;
  }
  py::list getParameterListString(Utils::ParametersBase*p)
  {
    std::vector<std::string> v;
    p->getParameterListString(v);
    py::list pylist = py::list();
    auto itE=v.end();
    for (auto it=v.begin();it!=itE;++it) {
      pylist.append(*it);
    }
    return pylist;
  }

  //PB struct to expose protected methods to python (solving overloading at the same time):
  struct PB : public Utils::ParametersBase {
    void apdv1(const std::string&n, double d) { addParameterDouble(n,d); }
    void apdv2(const std::string&n, double d, double l, double h) { addParameterDouble(n,d,l,h); }
    void apiv1(const std::string&n, int i) { addParameterInt(n,i); }
    void apiv2(const std::string&n, int i, int l, int h) { addParameterInt(n,i,l,h); }
    void apb(const std::string&n, bool b) { addParameterBoolean(n,b); }
    void aps(const std::string&n, const std::string& s) { addParameterString(n,s); }
  };

  void addParameterDouble_v1(Utils::ParametersBase*p, const std::string&n, double d) { static_cast<PB*>(p)->apdv1(n,d); }
  void addParameterDouble_v2(Utils::ParametersBase*p, const std::string&n, double d, double l, double h) { static_cast<PB*>(p)->apdv2(n,d,l,h); }
  void addParameterInt_v1(Utils::ParametersBase*p, const std::string&n, int i) { static_cast<PB*>(p)->apiv1(n,i); }
  void addParameterInt_v2(Utils::ParametersBase*p, const std::string&n, int i, int l, int h) { static_cast<PB*>(p)->apiv2(n,i,l,h); }
  void addParameterBoolean(Utils::ParametersBase*p, const std::string&n, bool b) { static_cast<PB*>(p)->apb(n,b); }
  void addParameterString(Utils::ParametersBase*p, const std::string&n, const std::string& s) { static_cast<PB*>(p)->aps(n,s); }
}

PYTHON_MODULE( mod )
{
  py::class_<Utils::ParametersBase>(mod, "ParametersBase")
    .def("setParameterDouble",&Utils::ParametersBase::setParameterDouble)
    .def("setParameterInt",&Utils::ParametersBase::setParameterInt)
    .def("setParameterBoolean",&Utils::ParametersBase::setParameterBoolean)
    .def("setParameterString",&Utils::ParametersBase::setParameterString)
    .def("lock",&Utils::ParametersBase::lock)
    .def("isLocked",&Utils::ParametersBase::isLocked)
    .def("getParameterDouble",&Utils::ParametersBase::getParameterDoubleNoLock)
    .def("getParameterInt",&Utils::ParametersBase::getParameterIntNoLock)
    .def("getParameterBoolean",&Utils::ParametersBase::getParameterBooleanNoLock)
    .def("getParameterString",&Utils::ParametersBase::getParameterStringCStrNoLock)
    .def("hasParameterDouble",&Utils::ParametersBase::hasParameterDouble)
    .def("hasParameterInt",&Utils::ParametersBase::hasParameterInt)
    .def("hasParameterBoolean",&Utils::ParametersBase::hasParameterBoolean)
    .def("hasParameterString",&Utils::ParametersBase::hasParameterString)
    .def("dump",&Utils::ParametersBase::dump)
    .def("dump",&Utils_ParametersBase_py::dump_0args)
    .def("dumpNoLock",&Utils::ParametersBase::dumpNoLock)
    .def("dumpNoLock",&Utils_ParametersBase_py::dumpNoLock_0args)
    .def("getParameterListDouble",&Utils_ParametersBase_py::getParameterListDouble)
    .def("getParameterListInt",&Utils_ParametersBase_py::getParameterListInt)
    .def("getParameterListBoolean",&Utils_ParametersBase_py::getParameterListBoolean)
    .def("getParameterListString",&Utils_ParametersBase_py::getParameterListString)
    .def("setIgnoreRanges",&Utils::ParametersBase::setIgnoreRanges)
    .def("addParameterDouble",&Utils_ParametersBase_py::addParameterDouble_v1)
    .def("addParameterDouble",&Utils_ParametersBase_py::addParameterDouble_v2)
    .def("addParameterInt",&Utils_ParametersBase_py::addParameterInt_v1)
    .def("addParameterInt",&Utils_ParametersBase_py::addParameterInt_v2)
    .def("addParameterBoolean",&Utils_ParametersBase_py::addParameterBoolean)
    .def("addParameterString",&Utils_ParametersBase_py::addParameterString)
    .def("noHardExitOnParameterFailure",&Utils::ParametersBase::noHardExitOnParameterFailure)
    .def("tieParameters",&Utils_ParametersBase_py::tieParameters_v1)
    .def("tieParameters",&Utils_ParametersBase_py::tieParameters_v2)
    .def("exposeParameter",&Utils::ParametersBase::exposeParameter)
    .def("exposeParameter",&Utils_ParametersBase_py::exposeParameter_2args)
    .def("exposeParameter",&Utils_ParametersBase_py::exposeParameter_3args)
    .def("exposeParameters",&Utils_ParametersBase_py::exposeParametersv1_1arg)
    .def("exposeParameters",&Utils_ParametersBase_py::exposeParametersv1_2args)
    .def("exposeParameters",&Utils_ParametersBase_py::exposeParametersv1_3args)
    //Todo: exposeParametersv2 with exclusions as well
    ;
}
