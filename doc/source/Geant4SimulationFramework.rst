*********
Old intro
*********

.. include:: wipwarning.rst

The Geant4 simulation framework is part of the `CodingFramework
<CodingFramework.html>`__ of the detector group. It includes functionality for
easy set up and launch of Geant4 jobs based on configurable geometry and
generator modules, and contains various functionality for physics choice,
visualisation, multiprocessing, random number handling, result persistification
and analysis, material configurability, etc.

.. note:: For a quick introduction to the framework see the slides from the tutorial given June 2015:
  `Download PDF <https://indico.esss.lu.se/event/335/attachments/2549/3795/dgcode_tutorial_june2015.pdf>`_
  In addition, find a few updates in the "Simulation Framework for the DG" talk given `here <https://indico.esss.lu.se/event/617/>`__ September 2016.


.. rubric:: Prerequisites
  :name: Geant4SimulationFramework-Prerequisites

To use the Geant4 simulation framework, you must install the base prerequisites of the `CodingFramework <CodingFramework.html>`__ and possibly also Geant4 (for simulating) and zlib (for reading/writing `Griff <Griff.html>`__ files). Make sure you have consulted `HowToInstallComputingPrereqs <https://confluence.esss.lu.se/display/DGCODE/HowToInstallComputingPrereqs>`__ for installation instructions as well as the instructions of the `CodingFramework <CodingFramework.html>`__ so you know the framework in which we work before proceeding. In particular, the following discussion assumes you know what is meant by a "package".

.. rubric:: What does the framework provide
  :name: Geant4SimulationFramework-Whatdoestheframeworkprovide

-  First and foremost it provides a common place to share and validate our geant4 work
-  Geometry and generators are written in C++ as separate modules

  -  Which parameters should be tunable/free is specified in the code

-  Control/steering module ("launcher") is configured in a small `python <https://confluence.esss.lu.se/display/DGCODE/PythonTutorial>`__ script:

  -  Choosing geometry/generator modules
  -  Optionally changing values of tunable parameters of geometry/generator
  -  Optionally change other defaults of the script

-  The script is then essentially a full-featured application. It provides a lot of functionality which can all be invoked by adding flags at the command line. Examples include:

  -  Specify number of events, initial random seed and number of processes in case multi-processing is desired.
  -  Select physics list (either reference G4 lists or our own custom lists which are also written as optional modules)
  -  Launch viewers or interactive Geant4 session.
  -  Control or enable/disable `Griff <Griff.html>`__ output.
  -  Allow tuning of geometry/generator parameters (useful for parameter scanning as well as playing around).

    -  This even includes materials through the "`Named materials <NamedMaterials.html>`__" mechanism.
    -  And `NCrystal <https://confluence.esss.lu.se/display/DGCODE/NCrystal>`__ materials enabling proper thermal neutron scattering in a wide range of materials.

.. rubric:: Where is the framework located
  :name: Geant4SimulationFramework-Whereistheframeworklocated

Although, it is not essentially at first for new users to know exactly where all the packages constituting our Geant4 simulation framework are located, an overview is given here for reference. All in all, it is spread out over a multitude of packages in the `CodingFramework <CodingFramework.html>`__:

-  O(20) packages under `Framework/G4 <https://github.com/mctools/dgcode/tree/main/packages/Framework/G4>`__ (including the vital interfaces in `G4Interfaces <https://github.com/mctools/dgcode/tree/main/packages/Framework/G4/G4Interfaces>`__ and control/steering/launcher class in `G4Launcher <https://github.com/mctools/dgcode/tree/main/packages/Framework/G4/G4Launcher>`__)

  -  Found here is also implementation of `NamedMaterials <NamedMaterials.html>`__, `Custom 3d viewer <Visualisation-of-Geant4-geometry-and-data.html>`__, `Particle generators for Geant4 <Particle-generators-for-Geant4.html>`__, `physics lists <G4-physics-lists.html>`__, ...

-  Packages implementing `Griff <Griff.html>`__ and related analysis utilities (under `Framework/Griff <https://github.com/mctools/dgcode/tree/main/packages/Framework/Griff>`__)
-  Packages implementing `NCrystal <https://confluence.esss.lu.se/display/DGCODE/NCrystal>`__ (under `Framework/NCrystal <https://github.com/mctools/dgcode/tree/main/packages/Framework/External/NCrystal>`__)
-  Project-specific packages providing various geometry modules and configuration scripts (normally in dedicated packages under `dgcode_projects/ <https://github.com/ess-dg/dgcode_projects>`__\ <projectname>)

Of course, some of the packages above are using other common packages not strictly related to the Geant4 simulation (such as `Core <https://github.com/mctools/dgcode/tree/main/packages/Framework/Core>`__, `Utils <https://github.com/mctools/dgcode/tree/main/packages/Framework/Utils>`__, `Framework/RandUtils <https://github.com/mctools/dgcode/tree/main/packages/Framework/Utils/RandUtils>`__, `SimpleHists <SimpleHists.html>`__ in `Framework/SimpleHists <https://github.com/mctools/dgcode/tree/main/packages/Framework/SimpleHists>`__, ...)

.. rubric:: Simple examples of how to use the framework
  :name: Geant4SimulationFramework-Simpleexamplesofhowtousetheframework

.. tip::
  In addition to the examples here, be sure to read about how to get quickly started by reading `How to start a new simulation project <How-to-start-a-new-simulation-project.html>`__.

Admittedly, some very extensive documentation of the simulation framework would be useful here, but partly due to time constraints and partly due to the fact that looking at actual examples is often the most useful approach to learning, here comes a "typical" example of a project using the framework (most actual projects are bound to be more complex of course).

In the packages `Projects/PlaneScatter/G4SimPlaneScatter <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter>`__ and `Projects/PlaneScatter/GriffAnaPlaneScatter <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/GriffAnaPlaneScatter>`__ are shown two relatively simple examples of how one can use the framework to setup and analyse a given situation in Geant4. They were written for a real use-case, in which some students at LU needed some simple G4 results for neutron scattering in a slab of material such as aluminium. It includes a geometry module, a simulation script, and an associated `Griff <Griff.html>`__ analysis application which runs through the generated data and produces histograms (`SimpleHists <SimpleHists.html>`__ in this case). Of course, not all projects will use `Griff <Griff.html>`__ files, and some projects will have to write their own generator modules and/or their own physics list module.

-  **Geometry module**: Defined as the class G4GeoPlaneScatter in the `G4SimPlaneScatter <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter>`__ package:

  -  `libinc/GeoPlane.hh <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter/libinc/GeoPlane.hh>`__ : Header file of the class. Notice that it inherits GeoConstructBase from the `G4Interfaces <https://github.com/mctools/dgcode/tree/main/packages/Framework/G4/G4Interfaces>`__ package.
  -  *`pycpp_GeoPlane/mod.cc <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter/pycpp_GeoPlane/mod.cc>`__* : Short boiler-plate file which makes the class available as a `python <https://confluence.esss.lu.se/display/DGCODE/PythonTutorial>`__ module named "G4SimPlaneScatter.GeoPlane"
  -  *`libsrc/GeoPlane.cc <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter/libsrc/GeoPlane.cc>`__* : Actual source file of the class. This is where we implement the geometry using standard Geant4 code.

    -  Notice how free/tunable parameters are declared and given default values in the constructor and used in the Construct method (where they might have changed to user-supplied values).
    -  Notice the convenience method "place" which can be used for simple geometries rather than the somewhat elaborate triplets of lines defining Shape/LogicalVolume/PhysicalVolume in most Geant4 code. Its usage is of course purely optional.

-  **Control script**: The script `scripts/sim <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter/scripts/sim>`__ in the `G4SimPlaneScatter <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter>`__ package is used to setup the G4 steering through a launcher object.

  - The launcher object is assigned an instance of our GeoPlane geometry as well
     as a generator (for this little project we did not need to write our own
     generator, and that is often the case even for more complicated setups).
  - Some free parameters are assigned new default values (here just for the
     generator, but in principle the same geometry module could also be used in
     different scripts with different defaults).
  - Other job settings can be given different defaults, here we just make sure
     that `Griff <Griff.html>`__ output will be in FULL mode and that the name
     of the output files will be plane.griff (or plane.N.griff in case of
     multi-processing). This will also be the place to change the default
     physics list.
  - Finally, the statement *"launcher.go()"* causes the framework to examine and
     react to the command line arguments of the user running the script.
  - After building, the script will be available as
     ``sb_g4simplanescatter_sim``. The first thing to do is to run
     ``sb_g4simplanescatter_sim -h`` to get a detailed description of how to use
     it. There are many options, a few immediately useful ones are:

    -  Running it without any arguments will launch a G4 simulation of just 10 events. Run with *-nNUMBER* to increase this, and run with *-jNUMBER* to enable multiprocessing.
    -  Run with --viewer to visualise the geometry with the experimental OpenSceneGraph-based `viewer <Visualisation-of-Geant4-geometry-and-data.html>`__.
    -  Run with - v to use an old-school G4 viewer. Run with -i to get the interactive G4 prompt.

-  **Griff analysis application**: Implemented as a little C++ application in the `GriffAnaPlaneScatter <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/GriffAnaPlaneScatter>`__ package:

  -  The application is defined in `app_simple/main.cc <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/GriffAnaPlaneScatter/app_simple/main.cc>`__, and after build it can be run as ``sb_griffanaplanescatter_simple <input_griff_file>`` and it produces a file with `histograms <SimpleHists.html>`__ planescatter.shist, which (if pylab is installed, cf. `HowToInstallComputingPrereqs <https://confluence.esss.lu.se/display/DGCODE/HowToInstallComputingPrereqs>`__) can be browsed interactively with ``sb_simplehists_browse planescatter.shist``.
  -  Note that `Griff <Griff.html>`__ analysis can be performed even on machines where Geant4 is not installed, which is a good reason to keep the analysis in a separate package than the geometry module and simulation script (since if Geant4 is missing, the `G4SimPlaneScatter <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter>`__ package gets disabled). A use-case would be if Geant4 is installed on a cluster where the simulation runs, but the analysis is carried out on a laptop without Geant4.
  -  Looking at the code in `app_simple/main.cc <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/GriffAnaPlaneScatter/app_simple/main.cc>`__, the structure is basically as follows:

    -  First a :sbpkg:`GriffDataReader<GriffDataRead/libinc/GriffDataReader.hh>` object is instantiated with the input file name (note that one can input many files if they can be defined via wildcards, e.g. ``sb_griffanaplanescatter_simple 'plane*.griff``)
    -  Histograms are "booked". Note how the parameters of the geometry and generator modules are available at this stage, and can be used to set up relevant histogram titles and ranges.
    -  Data selection filters are defined (this is optional).
    -  Finally, events in the file are looped through via the data reader loopEvents() method, and in each event pre-defined filters are used to extract just the relevant parts of the event in the form of instances of `Track <https://github.com/mctools/dgcode/blob/main/packages/Framework/Griff/GriffDataRead/libinc/Track.hh>`__, `Segment <https://github.com/mctools/dgcode/blob/main/packages/Framework/Griff/GriffDataRead/libinc/Segment.hh>`__, and `Step <https://github.com/mctools/dgcode/blob/main/packages/Framework/Griff/GriffDataRead/libinc/Step.hh>`__ classes. Once found, the pre-booked histograms are filled as desired. Note that instead of using the filters, one can also just start from the track objects (or primary track objects) on the data reader, and dive into the event through them.
    -  Finally, the histograms are saved to the output file.

.. rubric:: More elaborate examples of how to use the simulation framework
  :name: Geant4SimulationFramework-Moreelaborateexamplesofhowtousethesimulationframework

Detailed and instructive walk-through of a project is located at `ESSBoronTestCell <ESSBoronTestCell.html>`__. New users might find it helpful to get some automatically generated example code and start to edit that. For that approach, visit `How to start a new simulation project <How-to-start-a-new-simulation-project.html>`__.

.. rubric:: Dedicated documentations of parts of the simulation framework
  :name: Geant4SimulationFramework-Dedicateddocumentationsofpartsofthesimulationframework

Please find more documentation at:

-  `Griff <Griff.html>`__ - Description of the Griff data format and how to use it.
-  `NamedMaterials <NamedMaterials.html>`__ - Description of how one can specify materials from our central "material database" through string parameters.

  -  `NCrystal <https://confluence.esss.lu.se/display/DGCODE/NCrystal>`__ - Description of the NCrystal toolkit which provides correct neutron scattering in a wide range of materials.
  -  `IdealGasBuilder <IdealGasBuilder.html>`__ - How to easily specify custom gas mixtures.

-  `Particle generators for Geant4 <Particle-generators-for-Geant4.html>`__\
-  `Visualisation of Geant4 geometry and data <Visualisation-of-Geant4-geometry-and-data.html>`__ - Description of our custom OpenSceneGraph based viewer which is useful for geometry debugging and visualisation
-  `Extract and investigate cross-sections from Geant4 <Extract-and-investigate-cross-sections-from-Geant4.html>`__
-  `G4 physics lists <G4-physics-lists.html>`__ - Description of the custom physics lists provided in the framework.
-  `MCPL <MCPL.html>`__ - Description of the MCPL format for particle data and its usage in dgcode.
