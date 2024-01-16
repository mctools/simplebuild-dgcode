#include "Core/Python.hh"
#include "Utils/RefCountBase.hh"

namespace {
  //See https://github.com/pybind/pybind11/issues/114
  template < typename T>
  struct BlankDeleter
  {
    void operator()(T *) const {}
  };
}


PYTHON_MODULE( mod )
{
  py::class_<Utils::RefCountBase,std::unique_ptr<Utils::RefCountBase, BlankDeleter<Utils::RefCountBase>>>(mod, "RefCountBase")
    .def("refCount",&Utils::RefCountBase::refCount)
    .def("ref",&Utils::RefCountBase::ref)
    .def("unref",&Utils::RefCountBase::unref)
    .def("unrefNoDelete",&Utils::RefCountBase::unrefNoDelete)
    ;
}
