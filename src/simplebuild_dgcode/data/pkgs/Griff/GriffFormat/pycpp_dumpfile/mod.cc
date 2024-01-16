#include "Core/Python.hh"
#include "GriffFormat/DumpFile.hh"

PYTHON_MODULE( mod )
{
  mod.def("dumpFileInfo",&GriffFormat::dumpFileInfo,
          "dump content of G4 event file",
          py::arg("filename"),
          py::arg("brief")=false,
          py::arg("show_uncompressed_sizes")=false
          );

  mod.def("dumpFileEventDBSection",&GriffFormat::dumpFileEventDBSection,
          "binary dump to stdout of the content of the database section of evt at evtIndex position in Griff file",
          py::arg("filename"), py::arg("evtIndex")
          );

  mod.def("dumpFileEventBriefDataSection",&GriffFormat::dumpFileEventBriefDataSection,
          "binary dump to stdout of the content of the brief data section of evt at evtIndex position in Griff file",
          py::arg("filename"), py::arg("evtIndex")
          );

  mod.def("dumpFileEventFullDataSection",&GriffFormat::dumpFileEventFullDataSection,
          "binary dump to stdout of the content of the full data section of evt at evtIndex position in Griff file",
          py::arg("filename"), py::arg("evtIndex")
          );
}
