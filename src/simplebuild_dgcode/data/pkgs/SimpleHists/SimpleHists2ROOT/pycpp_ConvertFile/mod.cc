#include "Core/Python.hh"
#include "SimpleHists2ROOT/Convert.hh"
#include "SimpleHists/HistCollection.hh"

namespace SimpleHists_pycppcf {

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
    "Python module providing the convertToROOTFile functions from"
    " Convert.hh. If the convertToROOT functions are needed, one"
    " must use the sister module " sbld_stringify(PACKAGE_NAME) ".Convert instead.";
  mod.def("convertToROOTFile",&SimpleHists_pycppcf::convertToROOTFile_hc);
  mod.def("convertToROOTFile",&SimpleHists_pycppcf::convertToROOTFile_fn);
}

