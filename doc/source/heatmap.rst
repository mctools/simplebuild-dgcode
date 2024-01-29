.. _sbheatmap:

Heat Maps
=========

.. include:: wipwarning.rst

.. container::

  |image1|

"Heat-map" (really 3D histograms) information of any quantity can be easily
extracted from any sim-script. Either by adding lines inside the sim-script
itself, or simply by using the ``--heatmap`` command-line flag of the
sim-script. Assuming your simulation project was created as discussed :ref:`here
<sbnewsimproject>` and that it was named ``TriCorder``, the sim-script will be
named ``sb_tricorder_sim`` and you can get help concerning how to extract a
heatmap interactively:

.. include:: ../build/autogen_tricorder_simheatmaphelp.txt
  :literal:

FIXME

.. code-block:: sh

  Produce mesh3d files with extracted quantities from simulated particle
  steps, by supplying an argument with the following syntax:

    --heatmap="QUANTITY [where CONDITION] [to FILENAME]"

  QUANTITY is an expression based on the G4ExprParser.

  CONDITION is an optional filter expression based on the G4ExprParser.

  FILENAME is the name of the output file (defaults to "heatmap" if
  not provided). Postfix FILENAME with :(nx,ny,nz) to modify binning.

  Supplying --heatmap with no arguments implies --heatmap=step.edep

  Examples:

  1) Simply get a map of energy depositions:

    --heatmap=step.edep

  2) Same, but only for electrons and gamma and stored in edep_em.mesh3d:

    --heatmap="step.edep where trk.pdgcode==11||trk.is_photon to edep_em"

  3) Get approximate energy-flux map by calculating the average
    kinetic energy across the step and scaling with its length:

    --heatmap="0.5*(step.pre.ekin+step.post.ekin)*step.steplength"

Refer to :ref:`filter expressions <sbmcplfilterexpressions>` for a full list of
available parameters in the ``QUANTITY`` and ``CONDITION`` expressions.

Resulting heatmap files have extension ``.mesh3d`` and can be inspected
interactively via the ``sb_mesh3d_browse command``, leading to an interactive
display where one can view the 3D data projected onto any cartesian plane and
for selected slices of the third dimension. See the image above for an example.


.. |image1| image:: images/heatmap_example.png
   :width: 471px
   :height: 400px
