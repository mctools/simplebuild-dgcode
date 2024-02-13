************
Introduction
************

The Geant4 simulation framework which we call "dgcode" is implemented as a set
of `simplebuild <https://mctools.github.io/simplebuild>`__ packages
(specifically in the :sbpkg:`bundleroot::dgcode` bundle). Its :ref:`history
<sbhistory>` goes back to 2012, but in 2023/2024 it was cleaned up and
resurrected in its present form.

Most importantly, dgcode includes functionality for easy setting up and launching
of Geant4 simulation jobs based on configurable :ref:`geometry <sbgeo>` and
:ref:`generator <sbparticlegen>` modules, and contains various functionality for
:ref:`material definitions <sbmatdef>`, :ref:`physics lists <sbphyslist>`
choice, :ref:`visualisation <sb3dvis>`, multiprocessing, random number handling,
output persistification and analysis, etc.

Before starting to use dgcode, you should first of all make sure you have
:ref:`installed <sbinstall>` it, and secondly you should spend some time with
the simplebuild documentation at https://mctools.github.io/simplebuild, since
working with dgcode means working within the simplebuild build system. It will
eventually also be a good idea to dive into parts of the `Geant4 documentation
<https://geant4-userdoc.web.cern.ch/UsersGuides/AllGuides/html/>`__, in
particular the parts about `geometry definitions
<https://geant4-userdoc.web.cern.ch/UsersGuides/AllGuides/html/ForApplicationDevelopers/Detector/Geometry/geometry.html>`__
(note that for historical reasons geometry definition is often referred to as
"Detector definition" in Geant4).

Features
========

* Geometry is written in C++ as separate modules.

  * Which parameters should be tunable/free is specified in the code.
  * Parameters include :ref:`materials <sbmatdef>`, which are defined via simple
    strings (a :ref:`cookbook <sbmatcookbook>` of such strings makes material
    setup particularly easy).

* Generators are likewise written as separate modules with tunable parameters,
  but may also be written in C++. It is also often the case that a pre-existing
  generator can be used.
* Overall simulation control is configured in a small Python script known as a
  :ref:`sim-script <sbsimscript>`, in which one will:

  * Choose and configure :ref:`geometry <sbgeo>` and :ref:`particle
    generator <sbparticlegen>` modules.
  * Optionally change other defaults of the script, including tweaking the
    :ref:`simulation physics <sbphyslist>` and configure output data as needed.

* The :ref:`sim-script <sbsimscript>` is then essentially a full-featured
  application which can be launched from the command line. It provides a lot of
  functionality which can all be invoked by adding flags at the command
  line. Examples include:

  * Specify number of events, initial random seed and number of processes in
    case multi-processing is desired.
  * Select physics list (either reference G4 lists or custom lists which can
    also be written as optional modules).
  * `NCrystal <https://github.com/mctools/ncrystal/wiki>`__ is always
    enabled, ensuring high fidelity scattering cross sections for thermal
    neutrons.
  * Launch :ref:`3D viewers <sb3dvis>` or interactive Geant4 sessions.
  * Control output in :ref:`Griff <sbgriff>` or :ref:`MCPL <sbmcpl>` format, or
    collect custom :ref:`heat maps <sbheatmap>`.
  * Allow tuning of geometry and generator parameters, which is useful both for
    parameter scanning and for simply "playing around" and trying to understand
    a simulation.

* Post-simulation analysis can be done as desired, and might for instance
  include a Griff analysis written in C++, which outputs histogram data in
  SimpleHists files, usually followed by a final statistical analysis and
  plot-production in Python, where all the usual tools (e.g. `Matplotlib
  <https://matplotlib.org/>`__ and `SciPy <https://scipy.org/>`__) are
  available.

* Due in particular to being based on `simplebuild
  <https://mctools.github.io/simplebuild>`__, dgcode readily facilitates
  the creation of shared work between multiple related Geant4 projects within a
  group of people.

How to use the documentation
============================

New users will most likely benefit from first following the :ref:`installation
instructions <sbinstall>`, to ensure that dgcode and simplebuild are available
on the system. It is probably best to then spend a bit of time on the
`simplebuild documentation <https://mctools.github.io/simplebuild/>`__, focusing
at least on the introduction and usage examples. Then, it is probably time to
try to followed the instructions for :ref:`how to easily start a dgcode-based
simulation project <sbnewsimproject>`.

After these initial steps, it will be time to start diving into the more
detailed parts of the documentation, which you can find either from the sidebar
menu or via the :ref:`subjects overview page <sbsubjects>`. You can of course
also use the documentations search functionality in case you can not easily find
something.
