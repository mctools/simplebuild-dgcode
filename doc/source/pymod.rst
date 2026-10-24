.. _sbpymod:

***********************************************
Python modules with sbldpy (and how to migrate)
***********************************************

The compiled Python modules in dgcode are being moved from ``pycpp_*``
subdirectories (based on `pybind11 <https://pybind11.readthedocs.io/>`__) to
``pymod_*`` subdirectories, which are based on the much simpler `sbldpy
<https://mctools.github.io/simplebuild/sbldpy.html>`__ approach of
simplebuild. Modules based on sbldpy are faster to compile and do not depend on
the version of Python. New code should therefore use ``pymod_*``
subdirectories.

Existing ``pycpp_*`` modules in your own projects will keep working for now, and
the two kinds of modules can be used together (for instance, a geometry from a
``pycpp_*`` module can be used together with a generator from a ``pymod_*``
module in the same simulation). However, support for ``pycpp_*`` modules will
be removed in a future release, so you are encouraged to migrate your code as
described below.

Geometry, generator and filter modules
======================================

Most projects only have compiled Python modules which provide a geometry, a
particle generator or a step filter, like the
:sbpkg:`GeoSkeletonSP<G4GeoSkeletonSP/pymod__GeoSkeletonSP/geometry_module.cc>`
geometry used in new projects created with ``dgcode_newsimproject``. Such a
module can be migrated with a few simple steps, here shown for a geometry module
``GeoMyStuff`` in a package ``G4GeoMyStuff``:

#. Rename the directory ``G4GeoMyStuff/pycpp_GeoMyStuff/`` to
   ``G4GeoMyStuff/pymod__GeoMyStuff/`` (note the two underscores, which makes
   it a private module named ``_GeoMyStuff``).

#. In the code, include ``G4Interfaces/GeoConstructToPy.hh`` instead of
   ``G4Interfaces/GeoConstructPyExport.hh``, and replace:

   .. code-block:: c++

     PYTHON_MODULE( mod )
     {
       GeoConstructPyExport::exportGeo<GeoMyStuff>(mod, "GeoMyStuff");
     }

   with:

   .. code-block:: c++

     SBLDPY_MODULE( _GeoMyStuff ) { G4Interfaces::geoToPy<GeoMyStuff>( m ); }

#. Add a file ``G4GeoMyStuff/python/GeoMyStuff.py``, providing the Python
   module ``G4GeoMyStuff.GeoMyStuff`` with the usual ``create()`` function:

   .. code-block:: python

     from ._GeoMyStuff import create_handle as _create_handle
     from G4Interfaces.ToPy import component_creator as _component_creator
     create = _component_creator( _create_handle )

That is all. Python code using the module (e.g. ``geo =
G4GeoMyStuff.GeoMyStuff.create()``, setting parameters like ``geo.thickness_mm =
2.0``, or calling methods like ``geo.dump()``) works as before, and the C++ code
of the geometry class itself is unchanged.

Generators and step filters are migrated in exactly the same way, using
``G4Interfaces/ParticleGenToPy.hh`` and ``G4Interfaces::genToPy`` (instead of
``G4Interfaces/ParticleGenPyExport.hh`` and ``ParticleGenPyExport::exportGen``),
or ``G4Interfaces/StepFilterToPy.hh`` and ``G4Interfaces::filterToPy`` (instead
of ``G4Interfaces/StepFilterPyExport.hh`` and
``StepFilterPyExport::exportFilter``).

The objects created by such modules are instances of the Python class
``G4Interfaces.ToPy.Component``, which provides the same methods as the
pybind11 based objects. One difference is that they are not instances of the
pybind11 based classes, so code which checks the type of these objects (e.g.
with ``isinstance(geo,G4Interfaces.GeoConstructBase)``) must be updated.

Other modules
=============

Other modules can be migrated by following the `sbldpy documentation
<https://mctools.github.io/simplebuild/sbldpy.html>`__. Note in particular the
following differences compared to pybind11:

* Integer parameters must have the type ``std::int64_t``.
* Functions can not have default values of arguments or several overloads. If
  needed, put the compiled code in a private module (e.g. ``pymod__impl/``,
  providing the module ``_impl``) and add the missing features in Python code
  using it (e.g. ``python/mymodule.py``). For examples of this, see
  :sbpkg:`GriffFormat.dumpfile<GriffFormat/python/dumpfile.py>` or
  :sbpkg:`G4DataCollect<G4DataCollect/python/__init__.py>`.
* A ``pycpp__init/`` module was actually a module named ``_init``, imported
  automatically by the ``__init__.py`` file of the package. A ``pymod__init/``
  module directly provides the ``__init__.py`` file of the package, so if you
  also have a ``python/__init__.py`` file, use a private module (e.g.
  ``pymod__impl/``) instead.
