#include "Core/Python.hh"
#include "G4DataCollect/G4DataCollect.hh"

PYTHON_MODULE( mod )
{
  mod.def("installHooks",&G4DataCollect::installHooks,
          "Installs hooks necessary to record the output of the Geant4 simulation.",
          py::arg("outputFile"), py::arg("mode")="FULL"
          );
  mod.def("finish",&G4DataCollect::finish,"Uninstall hooks and close output file.");
  mod.def("setMetaData",&G4DataCollect::setMetaData);
  mod.def("setUserData",&G4DataCollect::setUserData);
}
