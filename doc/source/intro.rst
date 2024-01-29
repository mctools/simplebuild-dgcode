************
Introduction
************

.. include:: wipwarning.rst

The Geant4 simulation framework which we call "dgcode" is implemented as a set
of `simplebuild <https://mctools.github.io/simplebuild>`__ packages
(specifically in the :sbpkg:`bundleroot::dgcode` bundle). Its :ref:`history <sbhistory>` goes
back to 2012, but in 2023/2024 it was cleaned up and resurrected in its present
form.

Most importantly, dgcode includes functionality for easy seting up and launching
of Geant4 simulation jobs based on configurable geometry and generator modules,
and contains various functionality for material definitions, physics lists
choice, visualisation, multiprocessing, random number handling, output
persistification and analysis, etc.

Before starting to use dgcode, you should first of all make sure you have
:ref:`installed <sbinstall>` it, and secondly you should spend some time with
the simplebuild documentation at https://mctools.github.io/simplebuild, since
working with dgcode means using simplebuild-based workflow. It will eventually
also be a good idea to dive into parts of the `Geant4 documentation
<https://geant4-userdoc.web.cern.ch/UsersGuides/AllGuides/html/>`__, in
particular the parts about `geometry definitions
<https://geant4-userdoc.web.cern.ch/UsersGuides/AllGuides/html/ForApplicationDevelopers/Detector/Geometry/geometry.html>`__
(note that for historical reasons geometry definition is often refered to as
"Detector definition" in Geant4).

dgcode features
===============

* First and foremost it provides a common place to share and validate our geant4
  work.
* Geometry is written in C++ as separate modules.

  * Which parameters should be tunable/free is specified in the code.
  * Parameters include :ref:`materials <sbmatdef>`, which are defined via simple
    strings (a :ref:`cookbook <sbmatcookbook>` of such strings makes material
    setup particularly easy).

* Generators are written likewise written as separate modules with tunable
  parameters, but may also be written in C++. It is also often the case that a
  pre-existing generator can be used.
* Control/steering module ("launcher") is configured in a small Python script
  (called *the sim-script* of a given project):

  * Choosing and configuring geometry and generator modules.
  * Optionally change other defaults of the script.

* The sim-script is then essentially a full-featured application which can be
  launched from the command line. It provides a lot of functionality which can
  all be invoked by adding flags at the command line. Examples include:

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

How to use the documentation
============================

FIXME
