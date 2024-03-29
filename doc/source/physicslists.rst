.. _sbphyslist:

Physics lists
=============

Most users will not need to create custom physics lists, as dgcode by default
uses the ``QGSP_BIC_HP_EMZ`` reference list from Geant4, which provides a high
level of detail for most detector-related applications. Furthermore, integration
with `NCrystal <https://github.com/mctools/ncrystal/wiki>`__ is automatically
enabled whenever the user geometry contains NCrystal materials.

Occasionally it might be useful to be able to use a "sterile" physics list, in
which particles simply travel around without having any interactions. This might
for instance be useful for debugging purposes, or in case of wanting to avoid a
long initialisation time (e.g. when testing or debugging the geometry). For that
reason, dgcode provides a physics list ``PL_Empty`` which is designed to serve
that purpose (for convenience it can also be chosen under the alias ``empty``).

Note that you can always run the command ``sb_g4physicslists_showall`` to see a
list of all available lists (or alternatively supply the argument
``--showphysicslists`` to any :ref:`sim-script <sbsimscript>`).


Advanced information
--------------------

A somewhat outdated overview of physics lists from Geant4 4.9 can be found `here
<http://geant4.in2p3.fr/IMG/pdf_PhysicsLists.pdf>`__. A briefer more recent one
can be found `here
<https://indico.cern.ch/event/776050/contributions/3241826/attachments/1789270/2914266/ChoosingPhysLists.pdf>`__.
The latter slides incidentally contain (as the last slide) a reminder that special
cuts, in particular range cuts, might be applicable for a given application (see
also `here
<https://twiki.cern.ch/twiki/bin/view/Geant4/LoweAtomicDeexcitation>`__). In
general, if you need to tweak things like range cuts, etc., you must first
figure out the appropriate Geant4 UI commands which controls them. After that,
you can add the commands in your :ref:`sim-script <sbsimscript>`. Here are some
examples of how such commands might look:

.. code-block:: python

  launcher.cmd_preinit('/run/setCut 0.01 mm')
  launcher.cmd_preinit('/process/eLoss/StepFunction 0.1 0.001 um')
  launcher.cmd_preinit('/process/eLoss/StepFunctionMuHad 0.1 0.001 um')
  launcher.cmd_preinit('/process/eLoss/minKinEnergy 10 eV')

Note that as the name implies, the above commands will be invoked just *before*
the Geant4 run-manager object is initialised. If for some reason, you need to
ensure that they are instead invoked *after* the run-manager is initialised, you
must instead use ``launcher..cmd_postinit("...")``

If for some reason Geant4's thermal scattering models from T. Koi are required
(note: this is rare since NCrystal provides the same or more advanced
functionality), simply add a trailing "+TS" to the name of the physics list
(e.g. "QGSP_BIC_HP_EMZ+TS"). Likewise, special optical physics can be enabled
with "+OPTICAL", although this is not highly tested.

It is also possible to completely define your own physics list, as is often done
in standalone Geant4 applications. To do that, you must first of all know how to
define such a list in Geant4, and implement it in a shared library (i.e. in the
``libsrc/`` of a simplebuild package). In order to make your new list
automatically show up as an available physics list with the name
``PL_<yourphyslistname>``, you must perform two specific steps. The first step
is the addition of an empty file named
``plugin_g4physlist_<yourphyslistname>.txt`` in the ``data/`` directory of the
same package. This is needed solely as a way for your package to advertise the
existence of the physics list. The second step is to add a factory function
named ``sbldplugindef_g4physlist_<yourphyslistname>`` inside your shared
library. This factory function should actually create (i.e. ``return new
MyPhysList()``) the physics list and return a pointer to it, and it must be
enclosed in an ``extern "C" { ... }`` block.

For an example of how this is done, refer to the implementation of the
``PL_Empty`` physics list in the :sbpkg:`G4PhysicsLists` package: the file
:sbpkg:`data/plugin_g4physlist_Empty.txt<G4PhysicsLists/data/plugin_g4physlist_Empty.txt>`
declares that the package provides a physics list plugin named ``PL_Empty``, and
the ``sbldplugindef_g4physlist_Empty()`` function in
:sbpkg:`libsrc/PhysicsListEmpty.cc<G4PhysicsLists/libsrc/PhysicsListEmpty.cc>`
provides the call-back function which can be used to actually create such a
physics list when requested.

In general the topic of physics lists is a complicated one in Geant4. Feel free
to :ref:`reach out <sbcontact>` in case you need dgcode-specific advice for your
particular use case. But note that it might often be more appropriate to reach
out instead to the general Geant4 community, since the dgcode maintainers are
not experts in all the details.
