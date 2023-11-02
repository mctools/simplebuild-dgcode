#include "Core/Python.hh"
#include "SimpleHists2ROOT/Convert.hh"
#include "SimpleHists/HistCollection.hh"
#include "TH1D.h"
#include "TH2D.h"

#ifdef HAS_ROOT_PYTHON
#include "TPython.h"
#endif

namespace SimpleHists_pycpp {

  using PyObjReturnType = py::object;

  PyObjReturnType convertToPyROOT(const SimpleHists::HistBase*h, const char* root_name)
  {
#ifdef HAS_ROOT_PYTHON
    auto hr = SimpleHists::convertToROOT(h,root_name);
    bool python_owns = false;//NB: We used to have this set to true, but in
                             //ROOTv6 this leads to seg faults. Could
                             //investigate further, but for now we assume we can
                             //live with a few minor possible leaks.
    //NB: Method was called TPython::ObjectProxy_FromVoidPtr before ROOT 6.22.
    PyObject * raw_o = TPython::CPPInstance_FromVoidPtr(hr, hr->ClassName(), python_owns);
    auto handle = py::handle( raw_o );
    return handle.cast<py::object>();
#else
    (void)h;
    (void)root_name;
    return nullptr;
#endif
  }

  PyObjReturnType convertToROOT_1D(const SimpleHists::Hist1D*h, const char* rn) { return convertToPyROOT(h,rn); }
  PyObjReturnType convertToROOT_2D(const SimpleHists::Hist2D*h, const char* rn) { return convertToPyROOT(h,rn); }
  PyObjReturnType convertToROOT_Counts(const SimpleHists::HistCounts*h, const char* rn) { return convertToPyROOT(h,rn); }
  PyObjReturnType convertToROOT_Base(const SimpleHists::HistBase*h, const char* rn) { return convertToPyROOT(h,rn); }

  void convertToROOTFile_hc( const SimpleHists::HistCollection* hc,
                             const char* filename_root )
  {
    SimpleHists::convertToROOTFile(hc,filename_root);
  }

  void convertToROOTFile_fn( const char* filename_shist,
                             const char* filename_root )
  {
    SimpleHists::convertToROOTFile(filename_shist,filename_root);
  }

}

PYTHON_MODULE( mod )
{
  mod.doc() =
    "Python module providing the convertToROOT and convertToROOTFile"
    " functions from Convert.hh. It imports ROOT internally, which can"
    " be avoided by using the sister module " sbld_stringify(PACKAGE_NAME) ".ConvertFile"
    " instead, when only file-level manipulations are needed.";

  pyextra::pyimport("ROOT");

  mod.def("convertToROOT",&SimpleHists_pycpp::convertToROOT_1D);
  mod.def("convertToROOT",&SimpleHists_pycpp::convertToROOT_2D);
  mod.def("convertToROOT",&SimpleHists_pycpp::convertToROOT_Counts);
  mod.def("convertToROOT",&SimpleHists_pycpp::convertToROOT_Base);
  mod.def("convertToROOTFile",&SimpleHists_pycpp::convertToROOTFile_hc);
  mod.def("convertToROOTFile",&SimpleHists_pycpp::convertToROOTFile_fn);
}

