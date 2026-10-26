# Attic

Code which is no longer built or shipped, but is kept around as a starting point
in case it is ever needed again.

## SimpleHists2ROOT and SH2RTests

`SimpleHists2ROOT` provided utilities for converting between SimpleHists and
ROOT histograms, and `SH2RTests` contained its unit tests. They were
originally located in `src/simplebuild_dgcode/data/pkgs/SimpleHists/` and
`src/simplebuild_dgcode/data/pkgs_val/UnitTests/` respectively.

They were disabled when SimpleHists was migrated from pybind11 to sbldpy, since
the Python modules of `SimpleHists2ROOT` use pybind11 and the pybind11 based
SimpleHists classes. To resurrect them, the `pycpp_*` modules must be migrated
to sbldpy (or pure Python), and the packages moved back (replacing the
placeholder package `SimpleHists2ROOT` which currently produces an error
explaining the situation).
