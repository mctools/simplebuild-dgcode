.. _sbmatdef:

Material definition strings
===========================

.. include:: wipwarning.rst

Although it is certainly possible to define Geant4 materials via classical
Geant4 C++ code, materials in dgcode should mostly be defined via a single
material definition string. This mechanism, which in the past was also known as
the "NamedMaterial" mechanism, with several advantages:

#. It makes sure that the potentially cumbersome and error-prone task of
   defining and validating the needed Geant4 materials is kept separated from
   the task of implementing Geant4 geometries. That way, any development for
   materials will be immediately available to all existing and future
   geometries, and the task of developing new geometries is reduced
   significantly in scope and complexity.
#. By making any provided material selectable and configurable via a single
   string, it facilitates easy and flexible configuration of materials, allowing
   their type and significant properties to be manipulated from both compiled
   C++ code, interpreted Python code, or via command line and shell scripts.
#. It integrates directly with how materials are defined in `NCrystal
   <https://github.com/mctools/ncrystal/wiki>`_, allowing direct usage of these
   high-fidelity materials, and exposing all features of NCrystal directly.

Common examples
---------------

Before diving into the details, here are some examples of definitions of common materials


.. _sbmatdef_ncrystalcfgstrings:

NCrystal cfg-strings
--------------------

mention "MyPkg/myfile.ncmat".


https://github.com/mctools/ncrystal/wiki/Using-NCrystal#examples-of-configuration-strings
https://github.com/mctools/ncrystal/wiki/CfgRefDoc
https://github.com/mctools/ncrystal/wiki/Data-library

..
   Things with '::' or '@' or '<' characters: always NCrystal.

   Things with ';' chars
   
   If the first of '::',':',';' is a ':' char, assume classic mode. Also assume
   classic mode if one of ESS_Al, ESS_Ti, ESS_Cu, ESS_B4C, ESS_V, G4_Water, ...
   
   
   MIX:
   NISTNAME:temp_kelvin=...
   
   
   "NAME::*"
   "phases<*>"
   "*.ncmat[;...]"
   
   
   
   "Name:parameter1=value1:parameter2=value2:...:parameterN=valueN"
   `NCrystal cfg-strings
   <https://github.com/mctools/ncrystal/wiki/Using-NCrystal#uniform-material-configuration-syntax>`_
   can be used directly, allowing direct usage of these high-fidelity materials,
   with all the usual


NCrystal strings
----------------


https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html







.. rubric:: What does the NamedMaterialProvider do?
  :name: NamedMaterials-WhatdoestheNamedMaterialProviderdo?

The `NamedMaterialProvider <https://github.com/mctools/dgcode/blob/main/packages/Framework/G4/G4Materials/libinc/NamedMaterialProvider.hh>`__ is essentially a factory which creates and returns G4Material instances, all based on a configuration string supplied as input. The general format for such a configuration string is:

.. code-block:: sh

  "Name:parameter1=value1:parameter2=value2:...:parameterN=valueN"

Inside the `NamedMaterialProvider <https://github.com/mctools/dgcode/blob/main/packages/Framework/G4/G4Materials/libinc/NamedMaterialProvider.hh>`__, the *Name*, *parameters* and *values* are decoded and a corresponding G4Material is created and returned (via a cache in case the same material is requested again).

.. rubric:: How to use it
  :name: NamedMaterials-Howtouseit

.. rubric:: How it is used in a geometry implementation
  :name: NamedMaterials-Howitisusedinageometryimplementation

Simply define a string parameter and extract the resulting G4Material via getParameterMaterial(..) during geometry construction. Materials which should be hardcoded rather than configured by the user, can be created instead with getMaterial(...).

A very simple example of how this is done with a string parameter named "material" is shown in the example `GeoPlane.cc <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter/libsrc/GeoPlane.cc>`__ (which is discussed on the `Geant4SimulationFramework <Geant4SimulationFramework.html>`__ page)

.. rubric:: How it is used to configure a simulation
  :name: NamedMaterials-Howitisusedtoconfigureasimulation

Again, we will illustrate with a simple example. The `GeoPlane.cc <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter/libsrc/GeoPlane.cc>`__ geometry is used in `this simulation script <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter/scripts/sim>`__ which can be run via the command *ess_g4simplanescatter_sim*. Thus, from the command-line one can configure the material via:

.. code-block:: sh

  ess_g4simplanescatter_sim material="configuration_string_for_namedmaterialprovider"

Of course, in `the simulation script itself <https://github.com/ess-dg/dgcode_projects/tree/main/PlaneScatter/G4SimPlaneScatter/scripts/sim>`__, one could also have had a line:

.. code-block:: python

  geo.material="configuration_string_for_namedmaterialprovider"

.. rubric:: Inspect at commandline
  :name: NamedMaterials-Inspectatcommandline

To quickly see which G4Material would result from a given configuration string, one can use the *ess_g4materials_namedmat* command (NB, G4_Water is not actually the recommended way to simulate water now. Rather, it is recommended to do it with NCrystal):

.. code-block:: sh

  $> ess_g4materials_namedmat "G4_WATER:temp_kelvin=293.15"
  ===================================================================================
  Material defined by string "G4_WATER:temp_kelvin=293.15":
  Material: G4_WATER:temp_kelvin=293.15 H_2O   density:  1.000 g/cm3   RadL:  36.083 cm   Nucl.Int.Length:  75.517 cm   Imean:  78.000 eV
    --->  Element: H (H)   Z =  1.0   N =   1.0   A =   1.01 g/mole
          --->  Isotope:    H1   Z =  1   N =   1   A =   1.01 g/mole   abundance:  99.99 %
          --->  Isotope:    H2   Z =  1   N =   2   A =   2.01 g/mole   abundance:   0.01 %
            ElmMassFraction:  11.19 %  ElmAbundance  66.67 %
    --->  Element: O (O)   Z =  8.0   N =  16.0   A =  16.00 g/mole
          --->  Isotope:   O16   Z =  8   N =  16   A =  15.99 g/mole   abundance:  99.76 %
          --->  Isotope:   O17   Z =  8   N =  17   A =  17.00 g/mole   abundance:   0.04 %
          --->  Isotope:   O18   Z =  8   N =  18   A =  18.00 g/mole   abundance:   0.20 %
            ElmMassFraction:  88.81 %  ElmAbundance  33.33 %
  ===================================================================================

.. rubric:: Available materials and their parameters
  :name: NamedMaterials-Availablematerialsandtheirparameters

At the moment, the different types of materials can be created via the `NamedMaterialProvider <https://github.com/mctools/dgcode/blob/main/packages/Framework/G4/G4Materials/libinc/NamedMaterialProvider.hh>`__ as detailed in the sections below. Note that the following set of parameters are generally available for all types of materials:

-  temp_kelvin: Set the temperature of the material (default: 273.15)
-  density_gcm3 / density_kgm3 : Override the density of the material (units g/cm3 or kg/m3, default: Not set).
-  scale_density: Scales the density of the material by this factor (default: 1.0)

.. rubric:: Materials from the Geant4 NIST database (Name = G4_xxx)
  :name: NamedMaterials-MaterialsfromtheGeant4NISTdatabase(Name=G4_xxx)

Thankfully, a lot of useful materials are already implemented and shipped with Geant4 through the G4NistManager interface. To see the list of materials go to:

https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html?highlight=materials#nist-compounds

To select one of these materials, simply supply its name (like for instance "G4_H", "G4_CONCRETE", etc.) as the *Name* parameter of the configuration string.

For convenience the "G4\_" part is optional. Furthermore we provide a few aliases: "Vacuum"/"G4_Vacuum" is an alias for "G4_Galactic", "CO2" is an alias for "G4_CARBON_DIOXIDE" and "CH4" is an alias for "G4_METHANE". However, for CO2 and CH4, it is probably better to use the IdealGasBuilder (see below).

.. rubric:: NCrystal materials for improved crystalline neutron scattering (Name = NCrystal)
  :name: NamedMaterials-NCrystalmaterialsforimprovedcrystallineneutronscattering(Name=NCrystal)

Representing an improvement over pure Geant4 materials (or formerly used `NXSG4 <http://nxsg4.web.cern.ch/nxsg4/>`__ materials), NCrystal also enables simulation of thermal neutrons in crystals - with increased realism and support for both polycrystals/powders, single crystals, layered crystals (like pyrolythic graphite), liquids (including water and heavy water), and possibility to use not only in Geant4 but also standalone (python, C, C++, cmdline) or in McStas. Find links to more details on the `NCrystal <https://confluence.esss.lu.se/display/DGCODE/NCrystal>`__ page.

To configure an NCrystal material in Geant4,  one must provide an NCrystal configuration string (which can also be used outside Geant4 jobs). Such a string at the very least starts with the name of a material file (normally a .ncmat file), and optionally also can contain other parameters as desired (see https://github.com/mctools/ncrystal/wiki/Using-NCrystal for more details):

.. code-block:: python

  "NCrystal:cfg=Al_sg225.ncmat"              # polycrystalline aluminium using the Al_sg225.ncmat data file from the NCData package
  "NCrystal:cfg=Al_sg225.ncmat;temp=150K"     # same but at 150 kelvin
  "NCrystal:cfg=[Ge_sg227.ncmat;mos=40.0arcsec;dir1=@crys_hkl:5,1,1@lab:0,0,1;dir2=@crys_hkl:1,0,0:lab:1,0,0]"  #Ge-monochromator. Notice the usage of square brackets, needed since the string passed to cfg= contains colons.

By default, the crystallographic file (here Al_sg225.ncmat) is assumed to reside in `NCrystalRel/data <https://github.com/mctools/dgcode/tree/main/packages/Framework/External/NCrystal/NCrystalRel/data>`__ (visit the https://github.com/mctools/ncrystal/wiki/Data-library page for an overview of the files here, including plots, validation against experimental data, etc.). To specify a file residing elsewhere in the `CodingFramework <CodingFramework.html>`__, use the format ``PackageName/filename``, e.g. ``NXSLib/Al.nxs``.

You can investigate the cross-sections and scatterings of a given configuration with the ``ess_ncrystalrel_inspectfile`` script (note that the ``ess_ncrystal_inspectfile`` script is associated with the development version of NCrystal, see next section):

.. code-block:: sh

  ess_ncrystalrel_inspectfile "Al_sg225.ncmat;temp=500K"

Note that expert users can modify the base G4 material onto which NCrystal scattering is added, with the ``overridebaseg4mat`` parameter (must be a name in G4's NIST database), although this is not really recommended.

.. rubric:: Development version of NCrystal materials  (Name = NCrystalDev)
  :name: NamedMaterials-DevelopmentversionofNCrystalmaterials(Name=NCrystalDev)

Simply using the name "NCrystalDev" rather than "NCrystal", will use the NCrystal development code from Projects/NCrystal rather than the last stable version from Framework/NCrystal. Normally this should mainly be of interest to NCrystal developers and people interested in trying out new features not yet available in the finalised NCrystal releases at github.

.. note::
  To use NCrystalDev materials, one must make sure that the ``NCrystalPreview`` package is enabled. This is most easily done by adding the ``NCrystalPreview`` package as a requirement in your ``pkg.info`` file or (for a more temporary solution) enabling the NCrystal project (``dgbuild -pNCrystal``).

Note that attempting to use NCrystalDev materials with regular NCrystal materials in the same job will result in an error (and note that ESS_Al, ESS_Cu, etc. use regular NCrystal behind the scenes).

.. rubric:: Gas mixtures via IdealGasBuilder (Name=IdealGas)
  :name: NamedMaterials-GasmixturesviaIdealGasBuilder(Name=IdealGas)

As described on `IdealGasBuilder <IdealGasBuilder.html>`__, the IdealGasBuilder makes it possible to specify gas mixtures with a high degree of flexibility through one single Mixture formula. Features supported:

-  Single and multi component gasses.
-  Gas composition by-volume fractions or by-mass fractions.
-  Natural or custom isotopic abundances.
-  Automatic calculation of the unspecified variable of pressure, temperature and density (if less than 2 variables are specified, pressure and temperature will obtain STP conditions of 273.15K and 1atm).

In addition to a *formula* parameter for the gas composition, one can specify up to two state variables via:

-  temp_kelvin : The temperature of the material (units kelvin)
-  pressure_bar / pressure_atm : The pressure of the material (units bar or atmosphere)
-  density_gcm3 / density_kgm3 : The density of the material (units g/cm3 or kg/m3)

A few quick examples, showing configuration strings for a 70%-30% Ar-CO2 gas (at STP conditions), a 90%-10% (by-mass) BF3-CO2 mixture with boron enriched to 98% B10 at 2bar/300K and a pure helium-3 gas at 3bar:

.. code-block:: sh

  "IdealGas:formula=0.7*Ar+0.3*CO2"
  "IdealGas:formula=0.9*B{10|0.98}F3+0.1*CO2{bymass}:pressure_bar=2:temp_kelvin=300"
  "IdealGas:formula=He{3|1.0}:pressure_bar=3"

Refer to the documentation on `IdealGasBuilder <IdealGasBuilder.html>`__ for more details about how to create gas formulas.

.. rubric:: Particular custom materials
  :name: NamedMaterials-Particularcustommaterials

.. rubric:: Enriched Boron-Carbide (Name=ESS_B4C)
  :name: NamedMaterials-EnrichedBoron-Carbide(Name=ESS_B4C)

Similar to G4_BORON_CARBIDE from the G4 NIST database, but with an optional *b10_enrichment* parameter allowing for enrichment in the level of Boron-10. Note that the density of the material changes with the level of Boron-10 enrichment.

.. rubric:: Aliases for common materials including aluminium and copper (Name=ESS_Al or Name=ESS_Cu or Name=ESS_Ti)
  :name: NamedMaterials-Aliasesforcommonmaterialsincludingaluminiumandcopper(Name=ESS_AlorName=ESS_CuorName=ESS_Ti)

A few handy aliases exist for very common materials. Note, the entire string provided must be equal to the alias, and it is not possible to specify other parameters when using these. Currently there are three such aliases:

-  ``"ESS_Al"`` : Alias for ``"NCrystal:cfg=NCrystalRel/Al_sg225.ncmat"``
-  ``"ESS_Cu"`` : Alias for ``"NCrystal:cfg=NCrystalRel/Cu_sg225.ncmat"``
-  ``"ESS_Ti"`` : Alias for ``"NCrystal:cfg=NCrystalRel/Ti_sg194.ncmat"``
-  ``"ESS_V"`` : Alias for ``"NCrystal:cfg=NCrystalRel/TV_sg229.ncmat"``

.. rubric:: Polyethylene (Name=ESS_POLYETHYLENE)
  :name: NamedMaterials-Polyethylene(Name=ESS_POLYETHYLENE)

Polyethylene with Geant4's own thermal scattering by T. Koi enabled. Note that this thermal scattering physics only works correctly if using a physics list with thermal scattering enabled (for instance named like "QGSP_BIC_HP_EMZ+TS" instead of just "QGSP_BIC_HP_EMZ"). Note that is is planned to also provide Polyethylene and Plexiglass files for NCrystal, which would then work with any \_HP physics list.

.. rubric:: Gadolinium oxide (Name=Gd2O3)
  :name: NamedMaterials-Gadoliniumoxide(Name=Gd2O3)

Just that, with correct composition and density (no special physics modelling otherwise involved).

.. rubric:: NYLON-12 / Polyamide 2200 / PA 2200 (Name=NYLON-12)
  :name: NamedMaterials-NYLON-12/Polyamide2200/PA2200(Name=NYLON-12)

Several other Nylons are available in the Geant4 NIST database, but not this one.

.. rubric:: Mixtures: Quick and dirty mix of  any materials (Name=MIX)
  :name: NamedMaterials-Mixtures:Quickanddirtymixofanymaterials(Name=MIX)

A very crude way to quickly mix together two, three or four materials. Example, quickly create some concrete with an extra bit of copper:

.. code-block:: sh

  "MIX:comp1=CONCRETE:f1=0.99:comp2=Cu:f2=0.01"

The content of each component can be any other named material from this page (including the Geant4 NIST materials). If those materials are themselves needing parameters, it is probably necessary to enclose the entire compN parameter value in square brackets ( [ ] ), like:

.. code-block:: sh

  "MIX:comp1=[ESS_B4C:b10_enrichment=0.98]:f1=0.999:comp2=G4_Th:f2=0.001"

The state of the material (solid, gas or liquid) will be the same as that of the input components, meaning that they are required to all have the same state. If you wish to MIX materials with different states, you must add the parameter ":allowstatemix=true". The state of the resulting material will be that of comp1.

.. rubric:: Custom shielding materials (NAME="SHIELDING\_...")
  :name: NamedMaterials-Customshieldingmaterials(NAME="SHIELDING_...")

FIXME

The Neutron Optics & Shielding group have implemented a number of custom materials for their purposes, several of which works with the thermal scattering physics lists of Geant4. They all start with the prefix "SHIELDING\_" and a list of them can be found by looking at their definition in the source code in `Framework/G4/G4Materials/libsrc/ShieldingMaterials.cc <https://github.com/mctools/dgcode/blob/main/packages/Framework/G4/G4Materials/libsrc/ShieldingMaterials.cc>`__.
