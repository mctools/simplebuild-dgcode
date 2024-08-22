.. _sbmatcookbook:

Materials cookbook
==================

On this page we provide a small "cookbook" of material strings suitable for many
common use cases, in particular as regards materials for neutron detectors and
other components at neutron instruments. The discussion below assumes that
readers have first read :ref:`the general remarks about material
definitions <sbmatdef>`.

.. _sbmatsupportmats:

Support materials
-----------------

Although some support materials are more complicated (e.g. kapton, steel, ...),
many support materials (e.g. aluminium, copper, titanium, vanadium, alumina) can
for the most part be suitably modelled as singled-phased polycrystalline
materials under the powder-approximation using NCrystal -- thus enabling
modelling of thermal neutron physics with features like Bragg diffraction and
inelastic scattering on phonons. Therefore, one should simply find the relevant
``.ncmat`` file from the `NCrystal data library
<https://github.com/mctools/ncrystal/wiki/Data-library>`__ and use it in an
NCrystal cfg-string (more about searching for such files :ref:`here
<sbmatgeneral>`). The following table lists a few common examples. We will only
show how to modify the temperature and density of aluminium, but of course the
same methods applies to all the materials:

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Aluminium
     - ``stdlib::Al_sg225.ncmat``
   * - Aluminium with different temperature
     - ``stdlib::Al_sg225.ncmat;temp=200K``
   * - Aluminium with density overridden
     - ``stdlib::Al_sg225.ncmat;density=2.6gcm3``
   * - Aluminium with density scaled to 95%
     - ``stdlib::Al_sg225.ncmat;density=0.95x``
   * - Copper
     - ``stdlib::Cu_sg225.ncmat``
   * - Titanium
     - ``stdlib::Ti_sg194.ncmat``
   * - Alumina
     - ``stdlib::Al2O3_sg167_Corundum.ncmat``
   * - Vanadium
     - ``stdlib::V_sg229.ncmat``
   * - Kapton (see also :ref:`below <sbmathrich>`)
     - ``stdlib::Kapton_C22H10N2O5.ncmat``
   * - Iron (but see :ref:`below <sbmatironsteel>`)
     - ``stdlib::Fe_sg229_Iron-alpha.ncmat``
   * - Steel
     - :ref:`see below <sbmatironsteel>`

.. _sbmatironsteel:

Iron and steel
--------------

Although pure iron at atmospheric pressure and room temperature can be modelled
with ``stdlib::Fe_sg229_Iron-alpha.ncmat``, it is not often one finds exactly
this relatively soft material used in a neutron instrument. Instead, iron-rich
support materials used are primarily some sort of steel, and the exact
composition and material structure of steel is highly varied. While it is
envisioned that NCrystal will one day come with a small library of ready-made
steel materials (cf. `ncrystal#161
<https://github.com/mctools/ncrystal/issues/161>`__), this is not currently the
case. Instead, one can currently resort to a few different strategies:

- Use the ``G4_STAINLESS-STEEL`` material, which is a structure-less material
  with a particular composition defined in `Geant4's database
  <https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html>`__.
- Use either ``stdlib::Fe_sg229_Iron-alpha.ncmat`` or
  ``stdlib::Fe_sg225_Iron-gamma.ncmat``, and the
  `atomdb <https://github.com/mctools/ncrystal/wiki/CfgRefDoc>`__ keyword of
  NCrystal to inject small amounts of e.g. C, Cr, Cu, Mg, Ni, etc. as desired.
- Use the options for :ref:`multi-phased <sbmatmultiphase>` materials in case
  the material needs to be described with multiple phases.
- Feel free to posts comments on `ncrystal#161
  <https://github.com/mctools/ncrystal/issues/161>`__, to make the NCrystal
  developers aware of your use case, or provide other relevant feedback.

.. _sbmatgasmix:

Gas mixtures
------------

Based on NCrystal's intrinsic support for gas-mixture calculations, it is
easy to set up gas mixtures. The support is discussed in a more details `here
<https://github.com/mctools/ncrystal/wiki/Announcement-Release3.2.0>`__, with
just some hopefully self-explanatory examples listed in the following. Here
is first a few examples of single-component gasses:

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - CO₂ gas at P=1atm and T=20℃
     - ``gasmix::CO2``
   * - He gas at P=10bar and T=20℃
     - ``gasmix::He/10bar``
   * - BF3 gas enriched in Boron-10
     - ``gasmix::BF3/2atm/25C/B_is_0.95_B10_0.05_B11``
   * - Helium-3 gas
     - ``gasmix::He/10bar/25C/He_is_He3``

Multi-component gasses are defined in a similar manner, but obviously needs a
more complicated gas mixture specification. As an example, here are two ways to
specify the same 70%-30% (vol.) Ar-CO2 counting gas mixture at
:math:`\mathrm{P}=1.5\mathrm{atm}` and :math:`\mathrm{T}=250\mathrm{K}`. The
first example uses (implicit) volume fractions, and the second uses mass
fractions. It might be interesting to note that since
:math:`\mathrm{mass}(\mathrm{Ar})\approx\mathrm{mass}(\mathrm{CO}_2)`, the mass
and volume fractions are very similar. Also note, that under the ideal gas
assumption molar- and volume- fractions are always identical:

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Ar-CO2 mix (1)
     - ``gasmix::0.7xAr+0.3xCO2/1.5atm/250K``
   * - Ar-CO2 mix (2)
     - ``gasmix::0.679xAr+0.321xCO2/massfractions/1.5atm/250K``

A bit of humidity (i.e. contamination with water molecules), is also easily
added if you know the relative humidity:

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Ar-CO2 with 0.1% rel. humidity
     - ``gasmix::0.7xAr+0.3Ar/0.001relhumidity``

.. _sbmatair:

Air
---

Air is handled like any other gas mixture (see :ref:`above <sbmatgasmix>`), by
simply supplying the gas formula as ``air``:

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Air (1atm, 20℃)
     - ``gasmix::air``
   * - Air with 10% rel. humidity (1atm, 20℃)
     - ``gasmix::air/0.1relhumidity``
   * - Air (0.1atm, 150K)
     - ``gasmix::air/150K/0.1atm``

This usage of NCrystal's ``gasmix::air`` results in a material with around 10
different molecular components. If you find that the simulation speed in air is
limiting your simulation, and you don't actually really care much about the
precision of interactions in air, you could always use the Geant4 material
``G4_AIR`` instead, which (as per G4 v11.1.3) contains only O, N, Ar, and C
atoms.

Vacuum
------

For simulations it is often useful to be able to effectively "turn off"
interactions in a particular simulation volume. This is easily done in
practice by assigning a vacuum material to the volume:

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Vacuum
     - ``Vacuum`` (or ``void.ncmat`` if an NCrystal cfg-string is desired)

Note that the ``Vacuum`` material actually is a simple mapping to the
``G4_Galactic`` material from the Geant4 NIST database (although we could
also have picked the NCrystal ``void.ncmat`` material). If you want to
simulate the effects of a more realistic laboratory vacuum, you should refer
to the items below concerning :ref:`gas mixtures <sbmatgasmix>` or :ref:`air
<sbmatair>`, and simply assign a suitably low pressure or density.

.. _sbmatwater:

Water and heavy water
---------------------

Water (:math:`\mathrm{H}_2\mathrm{O}`) and heavy water
(:math:`\mathrm{H}^2_2\mathrm{O}=\mathrm{D}_2\mathrm{O}`) is currently modelled
in NCrystal via precomputed scattering kernels (a.k.a. large 2D tabulated values
of :math:`S(\alpha,\beta)` or :math:`S(q,\omega)`). This means that a given
`.ncmat` data file for water is currently only valid at one particular
temperature. To keep the NCrystal release size reasonable, only room temperature
water files are included with NCrystal itself:

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Water (room temperature)
     - ``stdlib::LiquidWaterH2O_T293.6K.ncmat``
   * - Heavy water (room temperature)
     - ``stdlib::LiquidHeavyWaterD2O_T293.6K.ncmat``

If needed, files for water at other temperatures than 293.6K can be found in
`the ncrystal-extra repository
<https://github.com/mctools/ncrystal-extra/tree/master/data/validated>`__.

.. _sbmatb4c:

Enriched boron-carbide (B4C)
----------------------------

Due to the strong absorption power of Boron-10, boron carbide (B4C) enriched in
the Boron-10 isotope, is often used as a converter in neutron detectors. As a
special feature, this material is directly supported in dgcode via the syntax
(note: these strings with ``MAT_B4C`` only work in dgcode, they are not NCrystal
cfg-strings):

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - B4C (natural, room temp.)
     - ``MAT_B4C``
   * - B4C (98% :math:`\mathrm{B}^{10}`, room temp.)
     - ``MAT_B4C:b10_enrichment=0.98``
   * - B4C (98% :math:`\mathrm{B}^{10}`, override temperature)
     - ``MAT_B4C:b10_enrichment=0.98:temp_kelvin=200.0``
   * - B4C (98% :math:`\mathrm{B}^{10}`, override density)
     - ``MAT_B4C:b10_enrichment=0.98:density_gcm3=2.3``
   * - B4C (98% :math:`\mathrm{B}^{10}`, scale density to 80%)
     - ``MAT_B4C:b10_enrichment=0.98:scale_density=0.8``

.. _sbmathrich:

Plastics and hydrogen-rich materials
------------------------------------

Many hydrogen-rich amorphous materials are included in the NCrystal data
library. They all support the usual NCrystal mechanisms for modifying
temperature and densities, but for brevity we only show-case it for polyethylene
in the list below. Due to the large differences in the densities of actual
incarnations of many of these materials (e.g. XPS versus EPS versions of
polystyrene) it is most likely a good idea to verify and possibly override the
density when using these to model a particular component. You can also read the
notes for a particular material by a command like ``nctool --extract
stdlib::Polystyrene_C8H8.ncmat | less`` (or simply find and click on the
material on the `NCrystal data library page
<https://github.com/mctools/ncrystal/wiki/Data-library>`__.

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Polyethylene (PE)
     - ``stdlib::Polyethylene_CH2.ncmat``
   * - Polyethylene (override temperature)
     - ``stdlib::Polyethylene_CH2.ncmat;temp=50C``
   * - Polyethylene (override density)
     - ``stdlib::Polyethylene_CH2.ncmat;density=0.8gcm3``
   * - Polyethylene (scale density to 90%)
     - ``stdlib::Polyethylene_CH2.ncmat;density=0.9x``
   * - Acrylic glass (Plexiglass, Lucite)
     - ``stdlib::AcrylicGlass_C5O2H8.ncmat``
   * - Epoxy resin
     - ``stdlib::Epoxy_Araldite506_C18H20O3.ncmat``
   * - Kapton
     - ``stdlib::Kapton_C22H10N2O5.ncmat``
   * - Nylon-11 / PA 11 / Rilsan
     - ``stdlib::Nylon11_C11H21NO.ncmat``
   * - Nylon-12 / PA 12 / PA 2200
     - ``stdlib::Nylon12_C12H23NO.ncmat``
   * - Nylon-6,10 / PA 610
     - ``stdlib::Nylon610_C16H30N2O2.ncmat``
   * - Nylon 66 / PA 66
     - ``stdlib::Nylon66or6_C12H22N2O2.ncmat``
   * - Polyether-ether-ketone / PEEK
     - ``stdlib::PEEK_C19H12O3.ncmat``
   * - Polycarbonate (Lexan)
     - ``stdlib::Polycarbonate_C16O3H14.ncmat``
   * - Polyester (PET)
     - ``stdlib::Polyester_C10H8O4.ncmat``
   * - Polylactide (PLA)
     - ``stdlib::Polylactide_C3H4O2.ncmat``
   * - Polypropylene (PP)
     - ``stdlib::Polypropylene_C3H6.ncmat``
   * - Polystyrene
     - ``stdlib::Polystyrene_C8H8.ncmat``
   * - Polyvinyl Chloride (PVC)
     - ``stdlib::PVC_C2H3Cl.ncmat``
   * - Rubber (polyisoprene, natural rubber)
     - ``stdlib::Rubber_C5H8.ncmat``

.. _sbmatbeamfilters:

Beam filters
------------

Here are some examples of common beam filters, used to filter out higher-energy
neutrons by scattering them out of the flight path:

 .. list-table::
    :header-rows: 1

    * - Material
      - String to use
    * - Beryllium filter
      - ``stdlib::Be_sg194.ncmat;temp=80K``
    * - Sapphire filter (simple+fast)
      - ``stdlib::Al2O3_sg167_Corundum.ncmat;bragg=0;temp=200K``

The recipe for the sapphire filter above simply uses the crude approximation
that the single-crystal sapphire filter is oriented so that no Bragg reflections
are possible from the direction of the incoming neutrons. For a more realistic
(and much more computationally intensive) approach, one must use the
:ref:`features for single crystal modelling <sbmatsinglecrystals>`. For a
detailed discussion of sapphire beam filters, refer furthermore to the dedicated
jupyter notebook which can be downloaded `here
<https://github.com/mctools/ncrystal-notebooks/blob/main/notebooks/misc/ncrystal_sapphire_filter.ipynb>`__
(with general instructions about how the notebooks can be run `here
<https://github.com/mctools/ncrystal-notebooks/>`__).

.. _sbmatgd2o3:

Gadolinium containing materials
-------------------------------

Materials with gadolinium in a crystal structure are on one hand easy to
model. This is because the absorption cross section tends to dwarf the
scattering cross section, making the actual material structure irrelevant for
anything except the density calculation.

On the other hand, if a more high-fidelity model is desired in which it is also
possible to model features like Bragg edges (for certain Gd isotopes they might
be relevant), one runs into the problem that the scattering lengths of the
neutron-Gd interactions might be energy dependent (i.e. some Gd isotopes have
low energy resonances), while the usual Bragg models assume constant scattering
lengths. A solution to this issue is being pursued in `ncrystal#147
<https://github.com/mctools/ncrystal/issues/147>`__.

For now, here are two NCrystal cfg-strings which can both be used to model
Gd2O3. One is an :ref:`unstructured solid <unstructuredmaterials>`, for which a
density must be explicitly provided (although Gd isotopes only have mass
differences of 5% so a value of 7.07 might be fine for many purposes), and in
which all scattering physics is modelled under a free-gas assumption. The other
is based on a particular crystalline structure, which has the advantage of
providing the density automatically, in addition to using more advanced neutron
scattering models (e.g. for Bragg diffraction):

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Unstructured Gd2O3 (explicit density)
     - ``solid::Gd2O3/7.07gcm3/Gd_is_0.9_Gd157_0.1_Gd155``
   * - Crystalline Gd2O3 (calculated density)
     - ``NCrystalExtra/Gd2O3_sg206.ncmat;atomdb=Gd:is:0.9:Gd157:0.1:Gd155``

One option which might be worth considering until `ncrystal#147
<https://github.com/mctools/ncrystal/issues/147>`__ is resolved, for modelling
single-phase materials containing highly absorbing Gadolinium is the following:
use NCrystal to *compose* the material, but not actually let NCrystal otherwise
take part in the actual modelling of the material (i.e. leaving that part
completely to Geant4). This can be done in dgcode via a special syntax:
``NCrystal:cfg=[<ncrystalcfgstr>]:g4physicsonly=1``, where ``<ncrystalcfgstr>``
is an NCrystal cfg-string defining the material. So for instance one might use
the string:

``NCrystal:cfg=[solid::Gd2O3/7.07gcm3/Gd_is_0.9_Gd157_0.1_Gd155]:g4physicsonly=1``

Feel free to :ref:`reach out <sbcontact>` in case you need advice for your
particular use case.

.. _unstructuredmaterials:

Unstructured materials
----------------------

Similarly to how one can define a :ref:`gas mixture <sbmatgasmix>` simply by
specifying its molecular composition along with a desired temperature and
pressure value, it is also possible to instead define a material simply by
providing its basic atomic composition, along with desired density and
temperature. Additionally, one must decide if the dynamics of the atoms in the
material is better approximated by freely moving atoms or atoms stuck in place
inside a solid. In either case, the syntax is the same, but one must use the
prefix ``freegas::`` or ``solid::`` as appropriate. Note that materials defined
in this manner *contain no information about material structure* and should thus
be mostly considered for materials where the material structure is not expected
to significantly alter the interactions with thermal neutrons. The best examples
of a material suitable for such modelling is a strongly absorbing materials
(like the :ref:`gadolinium containing materials <sbmatgd2o3>` discussed above),
although it might often be desirable to be able to model a material with the
free-gas model solely for debugging purposes. The following table shows a few
examples:

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Free-gas aluminium

       (for debugging purposes only)
     - ``freegas::Al/2.7gcm3;temp=200K``
   * - Enriched gadolinium oxide

       (see also :ref:`here <sbmatgd2o3>`)
     - ``solid::Gd2O3/7.07gcm3/Gd_is_0.9_Gd157_0.1_Gd155``
   * - Enriched boron carbide

       (see also :ref:`here <sbmatb4c>`)
     - ``solid::B4C/B_is_0.95_B10_0.05_B11/2.383gcm3``

.. _sbmatmultiphase:

Multi-phase materials
---------------------

For some use cases, definition of multi-phased materials might be important.
This might for instance be used to describe a multi-phased alloy, a crystalline
powder suspended in a liquid solution, an imperfectly packed material with void
areas, a material with density fluctuations, and so on. The multi-phase support
in NCrystal also serves as the foundation upon which SANS physics can be
supported (feel free to :ref:`reach out <sbcontact>` in case you need advice for
how to enable SANS physics).

Note that if you simply need to inject a bit of impurities into an otherwise
single-phased material, you should instead do it with the NCrystal ``atomdb``
keyword. For instance ``stdlib::Al_sg225.ncmat;atomdb=Al:is:0.995:Al:0.005:Cr``
produces a material with a typical aluminium lattice of atoms, but 0.5% of the
aluminium atoms are actually randomly switched with chromium atoms.

In general the NCrystal cfg-string syntax for defining a multi-phase material is
``phases<FRAC1*CFG1&..&FRACN*CFGN>[;COMMONCFG]``. Here, ``FRAC1`` is the
fraction of phase 1, which is defined by the NCrystal cfg-string ``CFG1``, and
so forth. The indicated fractions are assumed to be "by-volume" fractions, and
must sum to 1, and ``COMMONCFG`` contains cfg-parameters applied to all phases
(e.g. if ``COMMONCFG`` is ``;temp=200K``, all phases would change their
temperature -- which in the particular case of temperature is quite sensible).

Here are some examples of multi-phase materials:

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Quartz grains in heavy water:
     - ``phases<0.1*stdlib::SiO2-alpha_sg154_AlphaQuartz.ncmat&0.9*stdlib::LiquidHeavyWaterD2O_T293.6K.ncmat>``
   * - Enriched Gd2O3 grains mixed into epoxy
     - ``phases<0.05*solid::Gd2O3/7.07gcm3/Gd_is_0.9_Gd157_0.1_Gd155&0.95*stdlib::Epoxy_Araldite506_C18H20O3.ncmat>``

If your materials in the individual phases can *not* all be described with an
NCrystal cfg-string, the approach above will not work. Feel free to :ref:`reach
out <sbcontact>` in case you need advice for your particular use case.

.. _sbmatsinglecrystals:

Single crystals for monochromators and analysers
------------------------------------------------

Many monochromators and analysers used at neutron scattering instruments are
based on mosaic single crystal materials, and this is even the case for some
beam filters. In order to model these, one must use an appropriate model from
NCrystal. In most cases, this means using a Gaussian mosaicity distribution (see
instead :ref:`below <sbmatpg>` for pyrolitic graphite), which in NCrystal is
enabled simply by specifying not only a mosaic spread via the ``mos`` parameter,
but also the orientation of the preferred direction of the crystal. To do that,
one must provide two vectors in both the "laboratory-frame" and frame of the
crystal's unit cell. When the material is used in Geant4, the "laboratory-frame"
is taken to mean the local coordinate system of the Geant4 volume in which the
material in question is placed. The vectors are provided via the ``dir1`` and
``dir2`` parameters, whose exact syntax can be found `in the reference
documentation of NCrystal
<https://github.com/mctools/ncrystal/wiki/CfgRefDoc>`__. Other parameters of
relevance for single crystals are ``sccutoff`` and ``mosprec``.

In the case where one only knows the primary direction of a mosaic single
crystal, for example if a given monochromator has specified the plane normal
associated with a particular Bragg reflection but nothing else, the orientation
of the crystal is underspecified. If one nonetheless wants to proceed with
simulations, one can provide the known direction to ``dir1`` and a dummy
secondary direction to ``dir2``, and then also set ``dirtol=180deg``. Setting
``dirtol`` to this value means that NCrystal won't complain that the two
specified directions are not self-consistent, but instead silently "snap" the
provided ``dir2`` direction into a self-consistent position (the syntax for this
might eventually be simplified, see `ncrystal#155
<https://github.com/mctools/ncrystal/discussions/155>`__).

Here are examples for how a Germanium-511 monochromator or analyser can be
configured:

.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - Germanium-511 monochromator

       *FWHM mosaic spread:* :math:`20'`

       *Primary direction:* :math:`\bar{n}_{511}` along :math:`\hat{z}`

       *Secondary direction:* :math:`\bar{n}_{0\bar{1}1}` along :math:`\hat{y}`

     - ``stdlib::Ge_sg227.ncmat;mos=20.0arcmin;dir1=@crys_hkl:5,1,1@lab:0,0,1;dir2=@crys_hkl:0,-1,1@lab:0,1,0``

   * - Germanium-511 monochromator

       *FWHM mosaic spread:* :math:`20'`

       *Primary direction:* :math:`\bar{n}_{511}` along :math:`\hat{z}`

       *Secondary direction:* No direct choice.

     - ``stdlib::Ge_sg227.ncmat;mos=20.0arcmin;dir1=@crys_hkl:5,1,1@lab:0,0,1;dir2=@crys:1,0,0@lab:1,0,0;dirtol=180deg``

Single crystals are also discussed in the "Basic2" notebook at
https://github.com/mctools/ncrystal-notebooks.

.. admonition:: Recommended publications
  :class: tip

  | **If you use NCrystal to model single crystals, please cite:**
  | T. Kittelmann and X.-X. Cai, Comp. Phys. Commun 267 (2021) 108082,
  | `DOI 10.1016/j.cpc.2021.108082 <https://doi.org/10.1016/j.cpc.2021.108082>`__

.. _sbmatpg:

Pyrolytic graphite
------------------

Due to the special nature of the graphene sheets in graphite, the mosaic single
crystal graphite used in neutron instruments follow a different mosaic
distribution than the typical Gaussian one used :ref:`above
<sbmatsinglecrystals>`.

Thus, NCrystal supports a dedicated mosaic model with the special rotational
symmetry found in pyrolytic graphite (PG). This model is automatically enabled,
with the rotational symmetry axis set to the crystal's :math:`c`-axis, when the
relevant data file (``C_sg194_pyrolytic_graphite.ncmat``) is used as a single
crystal (if not used as a single crystal, it can be used as standard graphite).

The only thing to be aware of concerning the configuration, is that the primary
direction used with the ``dir1`` parameter should be parallel to the crystal's
:math:`c`-axis, which is for instance the case if using the
:math:`\bar{n}_{002}` normal to specify this direction. Secondly, although one
must still for technical reasons set ``dir2``, the value will not have any
effect in practice due to the rotational symmetry of the mosaic distribution --
and thus it is recommended to use the ``dirtol=180deg`` workaround to specify
it. The example below shows this in practice:


.. list-table::
   :header-rows: 1

   * - Material
     - String to use
   * - PG-002 monochromator

       *FWHM mosaic spread:* :math:`20'`

       *Primary direction:* :math:`\bar{n}_{002}` along :math:`\hat{z}`

     - ``stdlib::C_sg194_pyrolytic_graphite.ncmat;mos=20.0arcmin;dir1=@crys_hkl:0,0,2@lab:0,0,1;dir2=@crys_hkl:1,0,0@lab:0,1,0``

   * - Graphite powder

     - ``stdlib::C_sg194_pyrolytic_graphite.ncmat``

.. admonition:: Recommended publications
  :class: tip

  | **If you use NCrystal to model pyrolitic graphite, please cite:**
  | T. Kittelmann and X.-X. Cai, Comp. Phys. Commun 267 (2021) 108082,
  | `DOI 10.1016/j.cpc.2021.108082 <https://doi.org/10.1016/j.cpc.2021.108082>`__

Other materials
---------------

Many more materials than the ones mentioned in this cookbook are possible to
model with Geant4 and/or NCrystal: moderators (incl. with magnetic effects),
reflectors (incl. with SANS/nanodiamonds), optical materials, etc. Feel free to
:ref:`reach out <sbcontact>` in case you need advice.
