#include "Core/Python.hh"
#include "G4OSG/Viewer.hh"

namespace G4OSG {
  int run_viewer()
  {
    G4OSG::Viewer * viewer = new G4OSG::Viewer();
    int ec = viewer->run();
    if (ec)
      return ec;
    delete viewer;
    return 0;
  }
  void decode_py_list(py::list input, std::set<std::uint64_t>& o)
  {
    py::ssize_t n = py::len(input);
    for(py::ssize_t i=0;i<n;++i) {
      py::object elem = input[i];
      std::uint64_t evt;
      try {
        evt = elem.cast<std::uint64_t>();
      } catch ( py::cast_error& ) {
        throw std::runtime_error("Could not decode element in list as integer");
      }
      o.insert(evt);
    }
  }

  int run_viewer_with_griff(const char* griff_file, py::list pyevents)
  {
    std::set<std::uint64_t> events;
    decode_py_list(pyevents,events);
    G4OSG::Viewer * viewer = new G4OSG::Viewer(griff_file,true,events);
    int ec = viewer->run();
    if (ec)
      return ec;
    delete viewer;
    return 0;
  }
  int run_viewer_with_griff_1arg(const char* griff_file)
  { return run_viewer_with_griff(griff_file,py::list()); }

  int run_viewer_with_griff_no_geom(const char* griff_file, py::list pyevents)
  {
    std::set<std::uint64_t> events;
    decode_py_list(pyevents,events);
    G4OSG::Viewer * viewer = new G4OSG::Viewer(griff_file,false,events);
    int ec = viewer->run();
    if (ec)
      return ec;
    delete viewer;
    return 0;
  }
  int run_viewer_with_griff_no_geom_1arg(const char* griff_file)
  { return run_viewer_with_griff_no_geom(griff_file,py::list()); }

}

PYTHON_MODULE( mod )
{
  mod.def("run",&G4OSG::run_viewer);
  mod.def("run",&G4OSG::run_viewer_with_griff);
  mod.def("run",&G4OSG::run_viewer_with_griff_1arg);
  mod.def("run_nogeom",&G4OSG::run_viewer_with_griff_no_geom);
  mod.def("run_nogeom",&G4OSG::run_viewer_with_griff_no_geom_1arg);
}
