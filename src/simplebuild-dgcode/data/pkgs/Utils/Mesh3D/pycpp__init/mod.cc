#include "Core/Python.hh"
#include "Mesh/Mesh.hh"
#include <pybind11/numpy.h>

namespace {

  using PyNumpyArrayDbl = py::array_t<double,py::array::c_style>;

class py_Mesh3D {
public:
  py_Mesh3D(const std::string& filename)
  {
    Mesh::Mesh<3> mesh(filename);
    m_name = mesh.name();
    m_comments = mesh.comments();
    m_cellunits = mesh.cellunits();

    for ( auto& e : mesh.statMap() )
      m_stats[py::str(e.first)] = e.second;
    for (int i = 0; i < 3; ++i) {
      assert( mesh.filler().nCells(i) >= 0 );
      m_cells_n[i] = static_cast<std::size_t>(mesh.filler().nCells(i));
      m_cells_py.append(py::make_tuple(mesh.filler().nCells(i),
                                       mesh.filler().cellLower(i),
                                       mesh.filler().cellUpper(i)));
    }
    auto & datavect = mesh.filler().data();
    //init m_data as numpy array and access raw buffer:
    m_data = PyNumpyArrayDbl( {mesh.filler().nCells(0),mesh.filler().nCells(1),mesh.filler().nCells(2)} );
    std::size_t ntot = static_cast<std::size_t>( m_data.size() );
    (void)ntot;
    double * buf = m_data.mutable_data();
    assert( ntot == datavect.size() );
    assert(buf);
    size_t iblock = 0;
    size_t nblocks = datavect.blockCount();
    while ( (iblock = datavect.nextAllocBlock(iblock)) < nblocks) {
      auto blockdata = datavect.block(iblock);
      assert(blockdata);
      std::memcpy(&buf[datavect.block_size * iblock],&((*blockdata)[0]),sizeof(double)*blockdata->size());
      ++iblock;
    }
  }
  const char * getName() const { return m_name.c_str(); }
  const char * getComments() const { return m_comments.c_str(); }
  const char * getCellUnits() const { return m_cellunits.c_str(); }
  py::object getData() const { return m_data; }
  py::list getCellInfo_py() const { return m_cells_py; }

  py::dict getStats() const { return m_stats; }

  void print_summary() const {
    printf("Mesh3D:\n");
    printf("  Name       : %s\n",(m_name.empty()?"<none>":m_name.c_str()));
    printf("  Comments   : %s\n",(m_comments.empty()?"<none>":m_comments.c_str()));
    printf("  Cell units : %s\n",(m_cellunits.empty()?"<none>":m_cellunits.c_str()));
#if 0
    //Would be this simple if python 2.6 didn't produce slightly different
    //output of floats inside containers, breaking unit tests:
    std::string tmp = py::str(m_cells_py).cast<std::string>();
    printf("  Cells      : %s\n",tmp.c_str());
    tmp = py::str(m_stats).cast<std::string>();
    printf("  Stats      : %s\n",tmp.c_str());
#else
    //But we do it the hard way instead:
    std::string tmp[9];
    for (int i=0;i<9;++i)
      tmp[i] = py::cast<std::string>(py::str(m_cells_py[i/3].attr("__getitem__")(i%3)));//what a mess...
    printf("  Cells      : [(%s, %s, %s), (%s, %s, %s), (%s, %s, %s)]\n",
           tmp[0].c_str(),tmp[1].c_str(),tmp[2].c_str(),
           tmp[3].c_str(),tmp[4].c_str(),tmp[5].c_str(),
           tmp[6].c_str(),tmp[7].c_str(),tmp[8].c_str());
    py::list keys;
    for (auto& e : m_stats )
      keys.append( e.first );

    //py::list keys = m_stats.attr("keys")();
    keys.attr("sort")();
    printf("  Stats      : {");
    for (py::size_t i = 0; i<py::len(keys); ++i) {
      tmp[0] = keys[i].cast<std::string>();
      py::object o = m_stats[keys[i]];
      tmp[1] = py::str(o).cast<std::string>();
      printf("'%s': %s%s",
             tmp[0].c_str(),tmp[1].c_str(),
             (i+1==py::len(keys)?"}\n":", "));
    }
#endif
  }

  void print_cells(bool include_empty) const {
    std::size_t ntot = static_cast<std::size_t>( m_data.size() );
    (void)ntot;
    const double * buf = m_data.data();
    assert(buf);
    auto nx = m_cells_n[0];
    auto ny = m_cells_n[1];
    auto nz = m_cells_n[2];
    assert(std::size_t(nx*ny*nz)==ntot);
    for (std::int64_t ix = 0; ix < nx; ++ix)
      for (std::int64_t iy = 0; iy < ny; ++iy)
        for (std::int64_t iz = 0; iz < nz; ++iz) {
          double v = buf[ix*ny*nz+iy*nz+iz];
          if (v||include_empty)
            printf("  cell[%" PRId64 ",%" PRId64 ",%" PRId64 "] = %g\n",ix,iy,iz,v);
        }
  }
private:
  std::string m_name;
  std::string m_comments;
  std::string m_cellunits;
  py::dict m_stats;
  PyNumpyArrayDbl m_data;
  py::list m_cells_py;
  std::int64_t m_cells_n[3];
};

void py_Mesh3D_merge_files(std::string output_file, py::list input_files) {
  py::ssize_t n = py::len(input_files);
  if (n<2) {
    PyErr_SetString(PyExc_ValueError, "List of files to merge must be at least length 2");
    throw py::error_already_set();
  }
  std::string tmp = input_files[0].cast<std::string>();
  Mesh::Mesh<3> mesh(tmp);
  for (py::ssize_t i = 1; i < n; ++i) {
    tmp = input_files[i].cast<std::string>();
    mesh.merge(tmp);
  }
  mesh.saveToFile(output_file);
}
}

PYTHON_MODULE( mod )
{

  py::class_<py_Mesh3D>(mod,"Mesh3D")
    .def( py::init<std::string>() )
    .def_property_readonly("name",&py_Mesh3D::getName)
    .def_property_readonly("comments",&py_Mesh3D::getComments)
    .def_property_readonly("cellunits",&py_Mesh3D::getCellUnits)
    .def_property_readonly("data",&py_Mesh3D::getData)
    .def_property_readonly("cellinfo",&py_Mesh3D::getCellInfo_py)
    .def_property_readonly("stats",&py_Mesh3D::getStats)
    .def("dump_cells",&py_Mesh3D::print_cells)
    .def("print_summary",&py_Mesh3D::print_summary)
    ;

  mod.def("merge_files",&py_Mesh3D_merge_files);

}
