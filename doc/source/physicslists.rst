.. _sbphyslist:

Physics lists
=============

.. include:: wipwarning.rst

Most users will not need to create custom physics lists, as the default
``QGSP_BIC_HP_EMZ`` list provides a high level of detail for most
detector-related applications. Integration with `NCrystal
<https://github.com/mctools/ncrystal/wiki>`__ is automatically enabled whenever
the users geometry contain NCrystal materials. If Geant4's thermal scattering
models from T. Koi are required (such as if using the "ESS_POLYETHYLENE"
material), add a trailing "+TS" to the name of the physics list
(e.g. "QGSP_BIC_HP_EMZ+TS"). Likewise, special optical physics can be enabled
with "+OPTICAL".

A somewhat outdated overview of physics lists from Geant4 4.9 can be found at
http://geant4.in2p3.fr/IMG/pdf_PhysicsLists.pdf. A briefer more recent one is at
https://indico.cern.ch/event/776050/contributions/3241826/attachments/1789270/2914266/ChoosingPhysLists.pdf. The
latter slides incidently contain (as the last slide) a reminder that special
cuts, in particular range cuts, might be applicable for a given
application. These are normally set via the Geant4 UI command interface, which
in a standard simulation script in dgcode can be modified as (range cuts default
to 0.7mm, refer also to
https://twiki.cern.ch/twiki/bin/view/Geant4/LoweAtomicDeexcitation for
pecularities relating to electrons):

.. code-block:: python

  launcher.cmd_preinit('/run/setCut 0.01 mm')

Note: with Geant4 10.0.3 and the QGSP_BIC_HP list we used to have to provide the following settings in order to get correct energy deposition distributions (PHS) in b10-lined gaseous detectors:

.. code-block:: python

  launcher.cmd_postinit('/process/eLoss/StepFunction 0.1 0.001 um') #Not recommended/needed with _EMZ lists!!!
  launcher.cmd_postinit('/process/eLoss/minKinEnergy 10 eV')        #Not recommended/needed with _EMZ lists!!!

With the new precise EMZ physics lists, this is no longer required. If for some
reason one wishes to avoid the EMZ lists and stick with e.g. "QGSP_BIC_HP" in
Geant4 10.4.3 (this is not actually validated or recommended!), the lines should
become (more details `here <https://jira.esss.lu.se/browse/DGSW-305>`_):

.. code-block:: python

  launcher.cmd_preinit('/process/eLoss/StepFunction 0.1 0.001 um')      #Not recommended/needed with _EMZ lists!!!
  launcher.cmd_preinit('/process/eLoss/StepFunctionMuHad 0.1 0.001 um') #Not recommended/needed with _EMZ lists!!!
  launcher.cmd_preinit('/process/eLoss/minKinEnergy 10 eV')             #Not recommended/needed with _EMZ lists!!!

.. rubric:: List all available physics lists:
    :name: G4physicslists-Listallavailablephysicslists:
    :class: auto-cursor-target

Simply run the command ``sb_g4physicslists_showall`` (or supply the argument
``--showphysicslists`` to any sim script) to get a list printed of all available
physics lists. Any list prefixed with "ESS\_" is a custom list, created by an
expert user for various purposes (some are not well maintained or
documented). If the package in which it is built is disabled, it will not be
shown.

Some examples of custom physics lists (FIXME: Rename to ``Empty``):

- ``ESS_Empty``: An empty physics list (apart from some particles). This is very
   useful for testing, both because its almost instantaneous initialisation
   time, and because it is a quick way to "turn off" interactions of particles.

Find all custom physics lists (including those in disabled packages) with the command:

.. code-block:: sh

  sb --find pycpp_g4physlist_

That should also give some examples for inspiration, in case you need to create
your own physics list.
