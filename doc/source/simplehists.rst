.. _sbsimplehists:

SimpleHists
===========

.. include:: wipwarning.rst

This page concerns our custom histogram products for C++ and Python code,
implemented as the simplebuild package ``SimpleHists``.

|SimpleHists|

.. rubric:: Motivation
  :name: SimpleHists-Motivation

In addition to the obvious use-cases of histograms (e.g. estimations of data
probability density functions), histogram data types are also highly useful
tools for data reduction, as they have a limited size in memory and on disk no
matter how much data is filled in them. Implemented correctly, they can even
provide more functionality than just the counting of data-points in a certain
binning. Examples include: Handling of errors, weights and
normalisations. Collection of one-pass statistics such as unbinned mean and
variance. Association of metadata (title, axis labels). Utilities for on-disk
storage and quick plotting.

Histogram classes are for instance found in the `ROOT <http://root.cern.ch/>`__
framework which is common in high energy physics, but for python-centric
analyses based on e.g. "PyLab" (taken here to mean Numpy+SciPy+Matplotlib),
having a dependency on ROOT is a bit too cumbersome. However, one runs into the
problem that Pylab does not by default include histogram classes. Rather, very
basic histogramming functionality exists, but requires one to first collect all
data in one array thus defeating the data reduction purpose of histogram
classes.

Thus, it made sense to implement a few custom histogram classes in our
framework. Especially since features which are important to us could be ensured:
usage from both C++ and Python, integration with Pylab, persistification, quick
plotting, certainty that we can read important data in the future, and finally
the ability to merge data files collected in multiple concurrent jobs while
retaining correct statistical metadata.

.. rubric:: Features
  :name: SimpleHists-Features

-  Three feature-rich types of histograms: 1D, 2D and counter collection.
-  HistCollection which is a collection of histograms, each identified by a key.

   -  Simplifies user code.
   -  Can be easily merged with other collections
   -  Can be easily written to, or loaded from, a file (extension ``.shist``)

-  Histograms themselves are also easily (de) serialisable:

   -  To/from strings in both C++ and Python.
   -  Works with the standard Python ``pickle`` module.

-  Can be cloned, merged, normalised, scaled, integrated, ...
-  Command-line scripts for working with ``.shist`` files:

   -  ``sb_simplehists_browse``: Open up graphical browser (or all in terminal by specifying options).
   -  ``sb_simplehists_merge``: Merge contents in two ``.shist`` files into one. This is meant as an easy+reliable way to merge output done in multiprocessing environments (such as at the cluster).
   -  ``sb_simplehists_extract``: extract a subset of histograms from a file into a smaller one.
   -  ``sb_simplehists2root_convertfile``: For compatibility, convert histograms
      in ``.shist`` file to `ROOT <http://root.cern.ch/>`__ histograms and store
      in ``.root`` file. This requires ROOT to have been installed in the
      environment, which `might not be simple <https://github.com/conda-forge/root-feedstock/issues/214>`_.

-  Simple, fast on C++ side.
- Python side additionally features integration with Numpy arrays and matplotlib
  plotting.

.. rubric:: Example part 1 : Produce histograms in C++ code running on the cluster
  :name: SimpleHists-Examplepart1:ProducehistogramsinC++coderunningonthecluster

Imagine for instance the following to be part of some big event loop running on
the cluster (FIXME dynamic injection of the following example):

.. code-block:: C++

  SimpleHists::HistCollection hc;
  auto h_edep = hc.book1D("Energy deposited in counting gas",100,0.0,2000,"edep");
  h_edep->setXLabel("keV");
  auto h_scatterpos = hc.book2D("Scatter coordinates",100,0.0,2.0,100,0.0,2.0,"scatterpos");
  h_scatterpos->setXLabel("depth [meters]");
  h_scatterpos->setXLabel("height [meters]");
  //Some big loop:
  for (unsigned ievt=0; ievt < bignum; ++ievt) {
    // ...
    // All sorts of expensive stuff, for instance related to Geant4 simulations.
    // ...
    h_edep->fill(edep);
    h_scatterpos->fill(particle.x(),particle.y());
    // ...
  }
  hc.saveToFile("results.shist");

Afterwards one might for instance use the ``sb_simplehists_merge`` command to
merge the ``result.shist`` files from many different cluster jobs into
one. Thus, relevant data from many billions and billions of events are now all
present in a single small (tens of kilobytes) file which can be copied easily
down to ones laptop.

.. rubric:: Example part 2: Analysing the results on your laptop
  :name: SimpleHists-Examplepart2:Analysingtheresultsonyourlaptop

After having copied down the results.shist file to your laptop, the first thing
to do is to have a quick look inside. This is done by the command:

.. code-block:: sh

  sb_simplehists_browse results.shist

This opens up a graphical browser which can be used to quickly view the
histograms with various options for the presentations. At this stage it is
already possible to produce a few quick plots for a paper, talk or email.

For more advanced analysis, one can use Python and the plethora of utilities
available there (e.g. SciPy). Here is a small example of how one can get data out in formats
ready to input to the various pylab plotting routines:

.. code-block:: python

  import pylab as pl
  import SimpleHists as sh
  hc = sh.HistCollection('results.shist')
  h_edep = hc.hist('edep')
  #One can launch the quick interactive view for this histogram by:
  h_edep.plot()
  #But for advanced pylab analysis and plots you can ask for the contents
  #and bin edges in the same format as pl.histogram(..) would return:
  contents, edges = h_edep.histogram()
  #This can be used for custom analysis (using scipy fitting/interpolation
  #tools, making plots with analytical results on top, etc., etc.)
  #One can access other statistics as well of course:
  print 'edep variance =', h_edep.rms
  print 'mean edep     =', h_edep.mean

.. |SimpleHists| image:: images/Simplehists_preliminary_preview.png
   :width: 400px
