.. _sbgeo:

Geometry
========

.. include:: wipwarning.rst

In principle, a dgcode geometry module can use completely normal Geant4 code to
construct the geometry, inside the ``Construct()`` method of the associated
geometry class. The only constraint is that it must return the world volume (the
physical volume into which all other volumes are placed directly or indirectly).

.. code-block:: c++

  //Include whatever Geant4 header files here you want:
  #include "G4Box.hh"

  G4VPhysicalVolume* GeoTricorder::Construct()
  {
    // ... construct your Geant4 geometry in whatever manner you wish here...
    //
    // And return the world volume inside which all other volumes are placed:
    //
    return myWorld;
  }


It is beyond the scope of the present page to cover all aspects and
possibilities for how one can construct a geometry in Geant4, and it is
recommended to also spend some time reading about `geometry defintions in the
Geant4 documentation
<https://geant4-userdoc.web.cern.ch/UsersGuides/AllGuides/html/ForApplicationDevelopers/Detector/Geometry/geometry.html>`__
(note that for historical reasons geometry definition is often refered to as
"Detector definition" in Geant4). At the very least, it is important to have a
clear understanding of the difference between the Geant4 concepts of a *solid*
(a geometrical shape like a box or cylinder), a *logical volume* (a solid which
also has a material defined), and a *physical volume* (a logical volume which
has been placed into a mother logical volume in a particular position). It is
also important to understand how a geometry is built up in a tree structure,
with daughter volumes inside their mother volumes, and the world volume being at
the root of this tree.

While developing a geometry, you most certainly will want to visualise the
effect of the code you are developing. Visualisation of geometry can for
instance be performed by supplying a ``--viewer`` flag to any :ref:`sim-script
<sbsimscript>` or simply via the with the ``sb_g4utils_geodisplay``
command. Refer to :ref:`the dedicated page <sb3dvis>` for details about how to
use the visualisation tools.

As concerns creation of ``G4Material`` objects, dgcode provides a handy way to
set these up from simple strings (cf. :ref:`an intro here <sbmatdef>` and the
list of examples in the :ref:`cookbook <sbmatcookbook>`). Thus, assuming you
have found the relevant string to use for your material in the cookbook, you can
easily create a material:

.. code-block:: c++

  auto mat_Al = getMaterial("stdlib::Al_sg225.ncmat;temp=250K");
  auto mat_Gas = getMaterial("gasmix::0.7xAr+0.3xCO2/1.5atm/250K");
  //mat_Al and mat_Gas are now pointers to G4Material objects.

Unless you plan to define a completely hardcoded and static geometry, you most
likely wish to expose one or more parameters of your simulation. This could be
anything from the thickness of a particular component, the number of gadgets, or
even the material that goes inside some particular component. The way to do this
is to define the parameters and their default values in the constructor of your
geometry class, and then query them inside the ``::Construct()`` method as
needed. We can see an example of how this is done in the :ref:`TriCorder
<sbnewsimproject>` example:

.. literalinclude:: ../build/autogen_tricorder_geomod_wocomments.cc
  :language: c++

In addition to being able to define ``Double`` and ``String`` type parameters
like this, one can also add parameters of ``Int`` and ``Boolean`` types (a
material is basically just a string parameter). A few other more advanced
options exists. Interested users are referred to the
:sbpkg:`GeoConstructBase<G4Interfaces/libinc/GeoConstructBase.hh>`,
:sbpkg:`GeoBase<G4Interfaces/libinc/GeoBase.hh>` and
:sbpkg:`ParametersBase<Utils/libinc/ParametersBase.hh>` classes for the
details. Also note how the example above use the good practice of making the
units of a particular parameter be explicit by its name, and later on converting
the value by multiplication with the appropriate constant (e.g. when extracting
``sample_posz_mm`` we immediately multiply its value with ``Units::mm`` to make
sure the actual C++ variable is now in a unit compatible with Geant4 units.

One final thing to note in the above, is that in dgcode we have a convenience
method called ``place(..)``, which is used to directly combine a shape with a
material and a position. As can be seen in the implementation
:sbpkg:`here<G4Interfaces/libsrc/GeoBase.cc>`, this is really just wrapping
completely standard Geant4 code. Feel free to use this function or not as you
wish.

For completeness we show how some or all of the parameters might be modified in
a Python :ref:`sim-script <sbsimscript>`:

.. literalinclude:: ../build/autogen_tricorder_simscript_wocomments.py
  :language: python

Or they might be modified directly from the command line::

  $> sb_tricorder_sim material_sample='stdlib::Cu_sg225.ncmat' sample_radius_mm=7.0

