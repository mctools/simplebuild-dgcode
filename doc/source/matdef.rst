.. _sbmatdef:

Defining materials
==================

.. include:: wipwarning.rst

Although it is certainly possible to define Geant4 materials in dgcode via
classical Geant4 C++ code, materials in dgcode are instead mostly defined via a
simple material definition *string* (see the :ref:`geometry <sbgeo>` section for
how this is done in practice). This mechanism, which in the past was also known
as the "NamedMaterial" mechanism, has several advantages:

#. Defining materials is easier.
#. Materials can be easily modified from command line or Python code.
#. It integrates directly with how materials are defined in `NCrystal
   <https://github.com/mctools/ncrystal/wiki>`_, allowing direct usage of these
   high-fidelity materials, and exposing all features of NCrystal directly.

In the following, we will provide a few :ref:`general remarks <sbmatgeneral>`
about the content of these material definition strings and how to investigate
their effect. After that, most users will probably find the :ref:`"cookbook"
<sbmatcookbook>` of actual material examples to be the most useful reference.

.. _sbmatgeneral:

General remarks
---------------

The strings defining Most materials in typical dgcode projects will actually be
the same strings that are used with `NCrystal (aka "NCrystal cfg-strings)
<https://github.com/mctools/ncrystal/wiki/Using-NCrystal#uniform-material-configuration-syntax>`_,
so you might wish to consult the NCrystal documentation for those in the
`NCrystal wiki
<https://github.com/mctools/ncrystal/wiki/Using-NCrystal#uniform-material-configuration-syntax>`_. Most
of the entries in the :ref:`cookbook <sbmatcookbook>` are indeed such NCrystal
cfg-strings.

On that note, be aware that outside the :ref:`cookbook <sbmatcookbook>` you
can look for specific NCrystal materials both online in the `NCrystal data
library page <https://github.com/mctools/ncrystal/wiki/Data-library>`_, or in
your terminal via the ``nctool`` command by invoking ``nctool
--browse``. Combined with the unix ``grep`` command, you can use this for a
quick-and-dirty material search in the terminal. As an example, searching for
materials with aluminium with the command ``nctool -b|grep Al`` quickly reveals
that ``Al_sg225.ncmat`` is the correct data file name for aluminium. And this
filename is itself actually already a valid NCrystal cfg-string for room
temperature aluminium. If you wish a different temperature, it could easily be
achieved with a string like ``Al_sg225.ncmat;temp=200K``. If you do NOT find
your material in the NCrystal data library, you can always request help getting
it defined `here <https://github.com/mctools/ncrystal/issues/new>`__.

You can also read more about the available NCrystal parameters like
``temp=200K`` either `online
<https://github.com/mctools/ncrystal/wiki/CfgRefDoc>`__ or by invoking ``nctool
--doc`` or ``nctool --doc --doc``. Also, be aware that if you create your own
NCrystal data files in `NCMAT
<https://github.com/mctools/ncrystal/wiki/NCMAT-format>`__ format (for instance
according to the tutorials `here
<https://github.com/mctools/ncrystal-notebooks>`__), you can place them in
either your working directory or the `data/` directory of a simplebuild
package. It can then be used in material definitions via the name
``<pkgname>/<filename>`` syntax. In other words if you have added a file
``MyPkg/data/mycoolmat.ncmat``, you can load and use it in cfg-strings like
``MyPkg/mycoolmat.ncmat;temp=300K``.

Also, if you want to explicitly indicate that you *only* want the input file to
be taken from the official NCrystal data library, you can prefix the name with
``stdlib::`` (i.e. ``stdlib::Al_sg225.ncmat;temp=200K`` would use the
``Al_sg225.ncmat`` file from the official data library, even if you happened to
have a file named ``Al_sg225.ncmat`` lying around in your working
directory). For this reason, most of the examples in the :ref:`cookbook
<sbmatcookbook>` will use the ``stdlib::`` prefix.

In addition to NCrystal materials, you can also use any material from Geant4's
own `builtin database of elements and NIST compounds
<https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html>`_,
and even override their temperature or density, using dedicated keywords
``temp_kelvin`` (:math:`\mathrm{K}`), ``density_gcm3``
(:math:`\mathrm{g}/\mathrm{cm}^3`), ``density_kgm3``
(:math:`\mathrm{kg}/\mathrm{m}^3`), and ``scale_density``. For instance,
:math:`` xenon at :math:`200\mathrm{K}` and :math:`5.4\mathrm{kg}/\mathrm{m}^3`
could be modelled with a string ``G4_Xe;temp_kelvin=200.0;density_gcm3=5.4``
(although gases are in general more easily defined with the :ref:`NCrystal
syntax <sbmatgasmix>`.

.. tip::

   Note that while Geant4 NIST materials provide material compositions, they do
   in general do **not** contain information about the structure of the atoms
   inside the material. This structure is often very important when modelling
   scattering interactions with thermal neutrons, so in general only use Geant4
   NIST materials if your material is dominated by non-scattering effects
   (e.g. strongly absorbing materials) or you are not modelling thermal
   neutrons. If in doubt, prefer to use an NCrystal material.

Most users should not have a need for anything else than NCrystal cfg-strings
and Geant4 NIST materals, but a few other mostly deprecated options are
supported for backwards compatibility. As they are deprecated, they will not be
mentioned here or used in the :ref:`cookbook <sbmatcookbook>`, with the
exception of the special ``MAT_B4C`` keyword used for enriched :ref:`boron
carbide<sbmatb4c>`.


.. _sbmatstringusage:

Investigating materials
-----------------------

One advantage of defining materials via strings, is that it is possible to use
various tools to investigate properties of the materials, besides just using
them in a simulation. Below follows two recommended options for doing that in
dgcode.

Inspect via NCrystal utilities
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The material definitions which are also `NCrystal cfg-strings
<https://github.com/mctools/ncrystal/wiki/Using-NCrystal#uniform-material-configuration-syntax>`_,
can be investigated with NCrystal tools. The most easily used of these is the
command line tool ``nctool``, and the usage is very simple. First of all,
detailed thermal neutron cross sections and sample neutron scattering
distributions for the material can be found simply by feeding the material
string in question directly to ``nctool`` (remember the ``'`` quotes around
the string, or your terminal might misinterpret some of the special
characters inside it)::

  $> nctool 'stdlib::Al_sg225.ncmat;temp=250K'

Add a flag ``-a`` if you want absorption cross sections included as well, but
be aware that NCrystal only supports a simple ``1/v`` model of absorption
which will break down at higher neutron energies. When using the material
inside a Geant4 simulation, the more detailed absorption models by Geant4
will be used instead.

It is also possible to compare two materials, for instance to see how the
temperature affects the cross sections::

  $> nctool 'stdlib::Al_sg225.ncmat;temp=250K' 'stdlib::Al_sg225.ncmat;temp=500K'

To get some printed information about the material, rather than cross section
plots, you can use the ``--dump`` (or the shorter ``-d``) to get such
information printed:

.. include:: ../build/autogen_nctool_dump_example.txt
  :literal:

If you wish to read about how the material inside ``Al_sg225.ncmat`` was
defined, and perhaps read some comments about how the material was created
(or which publications to cite, etc.), you can find the file in the online
data library, or simple ask NCrystal to show you the contents (for this usage,
first remove parameters like ``;temp=250K``):

.. code-block::

  $> nctool --extract 'stdlib::Al_sg225.ncmat'

Many more options exists, to see them all run:

.. code-block::

  $> nctool --help

In case ``nctool`` does not exactly provide the plots you need, you might
instead find it useful to simply investigate the material via the NCrystal
Python API. A short example is found `here
<https://github.com/mctools/ncrystal/blob/master/examples/ncrystal_example_py>`__,
and Jupyter notebooks with much more details can be found `here
<https://github.com/mctools/ncrystal-notebooks/tree/main>`__ (do not forget to
scroll down and read the instructions).


Inspect via Geant4-based utilities
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The NCrystal-based approaches outlined in the previous section are mostly suited
for investigating scattering cross sections of thermal neutrons, and obviously
only works for materials which are defined by NCrystal cfg-strings. To
investigate other kinds of materials or cross sections, or the effects of
different :ref:`physics lists <sbphyslist>`, it is useful to be able to inspect
the cross sections directly as they actually manifest themselves in a Geant4
job. This is fortunately easily done in dgcode, and is documented :ref:`on this
dedicated page <sbxsect>`.

Additionally, if one is mostly interested in the basic composition of a given
material, one can simply feed the material string in question to the command
``sb_g4materials_dump``:

.. include:: ../build/autogen_g4matdump_g4stainlesssteel.txt
  :literal:
