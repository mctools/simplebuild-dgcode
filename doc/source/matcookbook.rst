.. _sbmatcookbook:

Materials cookbook
==================

.. include:: wipwarning.rst

* Support materials (aluminium, cobber, titanium, vanadium, alumina, kapton, ...):
   Such single-phase polycrystalline materials are best modelled under the
   powder-approximation using NCrystal, enabling modelling of thermal neutron
   physics with features like Bragg diffraction and inelastic scattering on
   phonons. Thus, simply find the relevant `.ncmat` file from the NCrystal data
   library and use it in an NCrystal cfg-string. Here are a few common examples,
   including Kapton for completeness (although it is actually modelled in a
   different way than the other materials here). We will only show how to modify
   the temperature and density of aluminium, but of course the same methods
   applies to all the materials:

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
      * - Kapton (see  also :ref:`below<sbmathrich>`)
        - ``stdlib::Kapton_C22H10N2O5.ncmat``
      * - Iron (but see :ref:`below<sbmatironsteel>`)
        - ``stdlib::Fe_sg229_Iron-alpha.ncmat``
      * - Steel
        - :ref:`see below<sbmatironsteel>`

.. _sbmatironsteel:

* Iron and steel
   Although pure iron at atmospheric pressure and room temperature can be
   modelled with ``stdlib::Fe_sg229_Iron-alpha.ncmat``, it is not often one
   finds exactly this relatively soft material used in a neutron
   instrument. Instead, iron-rich support materials used are primarily some sort
   of steel, and the exact composition and material structure of steel is highly
   varied. While it is envisioned that NCrystal will one day come with a small
   library of ready-made steel materials (cf. `ncrystal#161
   <https://github.com/mctools/ncrystal/issues/161>`__), this is not currently
   the case. Instead, one can currently resort to a few different strategies:

   - Use the ``G4_STAINLESS-STEEL`` material, which is a structure-less material
     with a particular composition defined in `Geant4's database
     <https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html>`_
   - Use either ``stdlib::Fe_sg229_Iron-alpha.ncmat`` or
     ``stdlib::Fe_sg225_Iron-gamma.ncmat``, and the
     `atomdb <https://github.com/mctools/ncrystal/wiki/CfgRefDoc>`__ keyword of
     NCrystal to inject small amounts of e.g. C, Cr, Cu, Mg, Ni, etc. as desired.
   - Use the options for :ref:`multi-phased <sbmatmultiphase>` materials in case
     the material needs to be described with multiple phases.
   - Feel free to posts comments on `ncrystal#161
     <https://github.com/mctools/ncrystal/issues/161>`__, to make the NCrystal
     developers aware of your use-case, or provide other relevant feedback.

.. _sbmatgasmix:

* Gas mixtures:
   Based on NCrystal's intrinsic support for gas-mixture calculations, it is
   easy to set up gas mixtures. The support is discussed in a more details `here
   <https://github.com/mctools/ncrystal/wiki/Announcement-Release3.2.0>`__, with
   just some hopefully self-explanatory examples listed in the following.  Here
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

   Multi-component gasses are defined in a similar manner, but obviously needs a
   more complicated gas mixture specification. As an example, here are two ways
   to specify the same 70%-30% (vol.) Ar-CO2 counting gas mixture at
   :math:`\mathrm{P}=1.5\mathrm{atm}` and :math:`\mathrm{T}=250\mathrm{K}`. The
   first example uses (implicit) volume fractions, and the second use mass
   fractions. It might be interesting to note that since
   :math:`\mathrm{mass}(\mathrm{Ar})\approx\mathrm{mass}(\mathrm{CO}_2)`, the
   mass and volume fractions are very similar. Also note, that under the ideal
   gas assumption molar- and volume- fractions are always identical:

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

* Air:
   Air is handled like any other gas mixture (see :ref:`above <sbmatgasmix>`),
   by simply supplying the gas formula as ``air``:

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
   different molecular components. If you find that the simulation speed in air
   is limiting your simulation, and you don't actually really care much about
   the precision of interactions in air, you could always use the Geant4
   material ``G4_AIR`` instead, which (as per G4 v11.1.3) contains only O, N,
   Ar, and C atoms.

* Vacuum:
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

* Water:
   Water (:math:`\mathrm{H}_2\mathrm{O}`) and heavy water
   (:math:`\mathrm{H}^2_2\mathrm{O}=\mathrm{D}_2\mathrm{O}`) is currently
   modelled in NCrystal via precomputed scattering kernels
   (a.k.a. large 2D tabulated values of :math:`S(\alpha,\beta)` or :math:`S(q,\omega)`). This means
   that a given `.ncmat` data file for water is currently only valid at one
   particular temperature. To keep the NCrystal release size reasonable, only
   room temperature water files are included with NCrystal itself:

   .. list-table::
      :header-rows: 1

      * - Material
        - String to use
      * - Water (room temperature)
        - ``stdlib::LiquidWaterH2O_T293.6K.ncmat``
      * - Heavy water (room temperature)
        - ``stdlib::LiquidWaterH2O_T293.6K.ncmat``

   If needed, files for water at other temperatures than 293.6K can be found in
   `the ncrystal-extra repository
   <https://github.com/mctools/ncrystal-extra/tree/master/data/validated>`__.

.. _sbmatb4c:

* Enriched boron-carbide (B4C):
   Due to the strong absorption power of Boron-10, boron carbide (B4C) enriched
   in the Boron-10 isotope, is often used as a converter in neutron
   detectors. As a special feature, this material is directly supported in
   dgcode via the syntax (note: these strings with ``MAT_B4C`` only work in
   dgcode, they are not NCrystal cfg-strings):

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

* Hydrogen-rich amorphous materials (including polethylene):
   Many hydrogen-rich amorphous materials are included in the NCrystal data
   library. They all support the usual NCrystal mechanisms for modifying
   temperature and densities, but for brevity we only show-case it for
   polyethylene in the list below. Due to the large differences in the
   densities of actual incarnations of many of these materials (e.g. XPS versus
   EPS versions of polystyrene) it is most likely a good idea to verify and
   possibly override the density when using these to model a particular
   component. You can also read the notes for a particular material by a
   command like ``nctool --extract stdlib::Polystyrene_C8H8.ncmat | less`` (or
   simply find and click on the material on the `NCrystal data library page
   <https://github.com/mctools/ncrystal/wiki/Data-library>`_.

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

* Beam filters:
   Here are some examples of common beam filters, used to filter out
   higher-energy neutrons by scattering them out of the flight path:

   .. list-table::
      :header-rows: 1

      * - Material
        - String to use
      * - Beryllium filter
        - ``stdlib::Be_sg194.ncmat;temp=80K``
      * - Sapphire filter (simple+fast)
        - ``stdlib::Al2O3_sg167_Corundum.ncmat;bragg=0;temp=200K``

  The recipe for the sapphire filter above simply uses the crude approximation
  that the single-crystal sapphire filter is oriented so that no Bragg
  reflections are possible from the direction of the incoming neutrons. For a
  more realistic (and much more computationally intensive) approach, refer to
  the dedicated jupyter notebook which can be downloaded `here
  <https://github.com/mctools/ncrystal-notebooks/blob/main/notebooks/misc/ncrystal_sapphire_filter.ipynb>`__
  (with general instructions about the notebooks `here
  <https://github.com/mctools/ncrystal-notebooks/>`__).

.. _sbmatgd2o3:


* :ref:`Gadolinium containing materials <sbmatgd2o3>`
   Materials with gadolinium in a crystal structure are on one hand easy to
   model. This is because the absorption cross section tends to dwarf the
   scattering cross section, making the actual material structure irrelevant for
   anything except the density calculation.

   On the other hand, if a more high-fidelity model is desired in which it is
   also possible to model features like Bragg edges (for certain Gd isotopes
   they might be relevant), one runs into the problem that the scattering
   lengths of the neutron-Gd interactions might be energy dependent (i.e. some
   Gd isotopes have low energy resonances), while the usual Bragg models assume
   constant scattering lengths. A solution to this issue is being pursued in
   `ncrystal#147 <https://github.com/mctools/ncrystal/issues/147>`__.

   For now, here are two NCrystal cfg-strings which can both be used to model
   Gd2O3. One is an unstructured solid, for which a density must be explicitly
   provided (although Gd isotopes only have mass differences of 5% so a value of
   7.07 might be fine for many purposes), and in which all scattering physics is
   modelled under a free-gas assumption. The other is based on a particular
   crystalline structure, which has the advantage of providing the density
   automatically, in addition to using more advanced neutron scattering models
   (e.g. for Bragg diffraction):

   .. list-table::
      :header-rows: 1

      * - Material
        - String to use
      * - Unstructured Gd2O3 (explicit density)
        - ``solid::Gd2O3/7.07gcm3/Gd_is_0.9_Gd157_0.1_Gd155``
      * - Crystalline Gd2O3 (calculated density)
        - ``NCrystalExtra/Gd2O3_sg206.ncmat;atomdb=Gd:is:0.9:Gd157:0.1:Gd155``

   One option which might be worth considering until `ncrystal#147
   <https://github.com/mctools/ncrystal/issues/147>`__ is resolved, for
   modelling single-phase materials containing highly absorbing Gadolinium is
   the following: use NCrystal to *compose* the material, but not actually let
   NCrystal otherwise take part in the actual modelling of the material
   (i.e. leaving that part completely to Geant4). This can be done in dgcode via
   a special syntax: ``NCrystal:cfg=[<ncrystalcfgstr>]:g4physicsonly=1``, where
   ``<ncrystalcfgstr>`` is an NCrystal cfg-string defining the material. So for
   instance one might use the string:

   ``NCrystal:cfg=[solid::Gd2O3/7.07gcm3/Gd_is_0.9_Gd157_0.1_Gd155]:g4physicsonly=1``

   Feel free to :ref:`reach out <sbcontact>` in case you need advice for your
   particular usecase.


.. _sbmatcustommixtures:

* :ref:`Custom mixtures <sbmatcustommixtures>`
   Fixme. ``solid::``, ``freegas::``, ... like ``gasmix::``.


.. _sbmatmultiphase:

* :ref:`Multi-phase materials <sbmatmultiphase>`
   For some use-cases, definition of multi-phased materials might be important.
   This might for instance be used to describe a multi-phased alloy, a
   crystalline powder suspended in a liquid solution, an imperfectly packed
   material with void areas, a material with density fluctuations, and so
   on. The multi-phase support in NCrystal also serves as the foundation upon which SANS
   physics can be supported (feel free to :ref:`reach out <sbcontact>` in case
   you need advice for how to enable SANS physics).

   Note that if you simply need to inject a bit of impurities into an otherwise
   single-phased material, you should instead do it with the NCrystal ``atomdb``
   keyword. For instance
   ``stdlib::Al_sg225.ncmat;atomdb=Al:is:0.995:Al:0.005:Cr`` produces a material
   with a typical aluminium lattice of atoms, but 0.5% of the aluminium atoms
   are actually randomly switched with chromium atoms.

   In general the NCrystal cfg-string syntax for defining a multi-phase material
   is ``phases<FRAC1*CFG1&..&FRACN*CFGN>[;COMMONCFG]``. Here, ``FRAC1`` is the
   fraction of phase 1, which is defined by the NCrystal cfg-string ``CFG1``,
   and so forth. The indicated fractions are assumed to by volume fractions, and
   must sum to 1, and ``COMMONCFG`` contains cfg-parameters applied to all
   phases (e.g. if ``COMMONCFG`` is ``;temp=200K``, all phases would change their
   temperature -- which in the particular case of temperature is quite
   sensible).

   Here are some examples of multi-phase materials:

   .. list-table::
      :header-rows: 1

      * - Material
        - String to use
      * - Quartz grains in heavy water:
        - ``phases<0.1*SiO2-alpha_sg154_AlphaQuartz.ncmat&0.9*LiquidHeavyWaterD2O_T293.6K.ncmat>``
      * - Enriched Gd2O3 grains mixed into epoxy
        - ``phases<0.05*solid::Gd2O3/7.07gcm3/Gd_is_0.9_Gd157_0.1_Gd155&0.95*Epoxy_Araldite506_C18H20O3.ncmat>``

   If your materials in the individual phases can *not* all be described with an
   NCrystal cfg-string, the approach above will not work. Feel free to
   :ref:`reach out <sbcontact>` in case you need advice for your particular
   use-case.

.. _sbmatsinglecrystals:

* Single crystals
   Fixme. Monochromators / analysers.
   (i.e. for monochromators, analysers and :ref:`beam filters <sbmatbeamfilters>`)

   ``Ge_sg227.ncmat;mos=20.0arcmin;dir1=@crys_hkl:5,1,1@lab:0,0,1;dir2=@crys_hkl:0,-1,1@lab:0,1,0``

   Single crystalline mosaic Germanium, which might be used in monochromators or
   analysers in a neutron instrument. The configuration of single crystals
   necessarily gets slightly more complicated, due to the need to specify the
   orientation of the crystal. This is done here by specifying that the normal
   of the (h,k,l)=(5,1,1) plane should point along the z-axis in the laboratory
   system, and the normal of the (h,k,l)=(0,-1,1) plane should point along the
   y-axis. By default, it results in an error if the angle between the specified
   directions in the crystal frame and the specified laboratory frame are not
   essentially identical. For simplicity, it is possible to increase this
   tolerance, when one is only interested in exact alignment of the primary
   direction (e.g. when one is only concerned with the primary reflection plane
   of a monochromator): specifying ``dirtol=180deg`` will relax the condition on the
   secondary direction maximally.




.. _sbmatpg:

* Pyrolytic graphite
   Fixme. Monochromators / analysers. Mention normal graphite.


* Other materials
   Many more materials than the ones mentioned in this cookbook are possible to
   model with Geant4 and/or NCrystal: Moderators (incl. with magnetic effects),
   reflectors (incl. with SANS/nanodiamonds), optical materials, etc. Feel free
   to :ref:`reach out <sbcontact>` in case you need advice.



