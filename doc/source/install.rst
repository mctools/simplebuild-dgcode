.. _sbinstall:

************
Installation
************

.. include:: wipwarning.rst

The simplebuild system and dgcode is supported only on unix systems (macOS and Linux),
although it most likely will also work on Windows under the WSL (Windows
Subsystem for Linux) with a virtual Ubuntu installation. It is used exclusively
by entering shell commands in a terminal interface, so be sure you are familiar
with such command line interfaces.

Install via conda
=================

..
  Fixme: add conda badges

The recommended and easiest way to install dgcode, is by creating an
appropriate conda environment based on the `conda-forge
<https://conda-forge.org/>`__ channel, in which all the dependencies are
included. The package for dgcode itself, must still be installed via the PyPI
(pip) package for the time being, although that will change at some point in the
future (cf. `#9
<https://github.com/mctools/simplebuild-dgcode/issues/9>`__). The recommended
conda environment ``.yml`` file to use to create the environment is the
following:

.. include:: ../../resources/conda_dgcode.yml
  :literal:

You can download the above recipe file here: :download:`conda_dgcode.yml
<../../resources/conda_dgcode.yml>`.

To use it, you must first install conda. Instructions for how to do that is
beyond the scope of the present documentation, but in general this can be done
in a variety of ways (installing Miniforge, Miniconda, Anaconda, or even via
Homebrew). If you don't have conda installed already and do not have any other
reason for a preference, we would recommend to use `Miniforge
<https://github.com/conda-forge/miniforge>`__ since it is light-weight and
supposedly has the fewest legal concerns.

After you have conda installed, download :download:`conda_dgcode.yml
<../../resources/conda_dgcode.yml>` and run the command::

  conda env create -f conda_dgcode.yml

Do not forget that you must activate your newly created environment before using
it for the first time in a given terminal session::

  conda activate dgcode


Alternatives for experts
========================

The conda recipe above is intended to give a self-contained and reproducible
environment with not only dgcode itself, but also any required tools like
`simplebuild <https://mctools.github.io/simplebuild>`, a Python interpreter and
all the necessary build tools. For special advanced use-cases, experts might
simply want to add the code itself into an environment where they otherwise have
ensured that all of these third-party tools are already available. In such a
case, one can simply install dgcode via ``pip``, either via a PyPI package
(current version |pypistatus_simplebuilddgcode|_)::

  python3 -mpip install simple-build-dgcode

Or, directly from the latest dgcode sources at GitHub::

  python3 -mpip install git+https://github.com/mctools/simplebuild-dgcode

With this latter approach, one can even install a specific commit id, branch, or
tag by appending ``@<gitid>`` to the URL in the last command. For instance::

  python3 -mpip install git+https://github.com/mctools/simplebuild-dgcode@some_experimental_branch

.. |pypistatus_simplebuilddgcode| image:: https://img.shields.io/pypi/v/simple-build-dgcode.svg
.. _pypistatus_simplebuilddgcode: https://pypi.org/project/simple-build-dgcode


Verifying an installation
=========================

As a very basic verification of a simplebuild installation, one can create a
simple simplebuild project and launch a few basic unit tests from the
:sbpkg:`bundleroot::dgcode_val` bundle (you can remove the leftover ``sbverify``
directory afterwards):

.. code-block::

   FIXME todo

The important thing to notice here is that several unit tests were launched, and
the message ``All tests completed without failures!`` tells us that they all
completed without problems.
