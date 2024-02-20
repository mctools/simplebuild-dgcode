.. _sbsimscript:

Sim-scripts
===========

In dgcode the term "sim-script" is short for "simulation-script" and refers to
the command-line scripts which are used to launch the simulations using a
:sbpkg:`G4Launcher` instance. They generally have the following structure (here
using a sim script from the :ref:`TriCorder example <sbnewsimproject>`):

.. literalinclude:: ../build/autogen_tricorder_simscript_wocomments.py
  :language: python

In other words, a geometry module and a generator module are loaded and
configured, before their combination is used to initialise a ``G4Launcher``
instance (here named ``launcher``). The ``G4launcher`` object can then itself be
configured, before ultimately its ``.go()`` method is invoked as the last line
in the file. At this point, command-line arguments of the script are
investigated, and some sort of action happens. This is usually to launch the
simulation, but might also be used to for instance query some details of the
setup, launch a :ref:`visualisation <sb3dvis>`, or something else. Since the
script itself in the :ref:`TriCorder example <sbnewsimproject>` is named ``sim``
and placed into the ``scripts`` folder of the ``TriCorder`` package, the
simplebuild mechanics means that the command which is ultimately used to invoke
the script is called ``sb_tricorder_sim``. It comes with built-in documentation
of how to use it, accessible by supplying the ``-h`` or ``--help`` flag:

.. literalinclude:: ../build/autogen_tricorder_sim_help.txt

Here is a bit of advice concerning when to typically use the options above:

* Notice in particular the ``-n`` and ``-j`` options, which control the number
  of events to simulate and how many processes to use (i.e. if you have a 4-core
  machine with hyperthreading enabled, it makes sense to use ``-j8``. If
  combining data from multiple runs, you should remember to use a different
  random seed in each job by using ``-s``.

* When debugging your geometry setup, you might want to use ``-g``, and
  ``t``. And most certainly you want to :ref:`visualise <sb3dvis>` the geometry
  with ``--viewer``.

* When debugging your particle generator setup, you might want to use ``-p``, or
  :ref:`visualise <sb3dvis>` particles with ``--dataviewer`` and
  ``--aimdataviewer``.

* If you want to quickly see what happens in a particular event, you can use
  ``-r`` (supply it once or multiple times). You might also want to use ``-n1 -s
  <SEEDVALUE>`` to start the simulation at a particular event. You can of course
  also use a subsequent :ref:`Griff <sbgriff>` analysis to investigate events
  programmatically.

* To control the output of the simulation, you can use ``-o`` and ``-m`` for
  :ref:`Griff <sbgriff>` output (disable it with ``-o none``), ``--mcpl`` for
  :ref:`MCPL <sbmcpl>` output, or ``--heatmap`` to collect :ref:`heat maps
  <sbheatmap>`.

* To modify and investigate physics, you can use ``-x`` to extract relevant
  :ref:`cross section files <sbxsect>`, or use ``--showphysicslists`` and ``-l``
  to control the physics list.
