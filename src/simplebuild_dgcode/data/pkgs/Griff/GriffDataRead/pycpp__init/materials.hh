namespace GDR_material {
  namespace {
    using Material = GriffDataRead::Material;
    using Element = GriffDataRead::Element;
    using Isotope = GriffDataRead::Isotope;

    py::object py_get_elem_list( const Material*m )
    {
      py::list l;
      assert(m);
      const unsigned n = m->numberElements();
      for ( unsigned i = 0; i < n; ++i ) {
        auto elem = m->getElement(i);
        l.append( py::cast( elem ) );
      }
      return l;
    }
    py::object py_get_isotope_list( const Element*elem )
    {
      py::list l;
      assert(elem);
      const unsigned n = elem->numberIsotopes();
      for ( unsigned i = 0; i < n; ++i ) {
        l.append( py::cast( elem->getIsotope(i) ) );
      }
      return l;
    }

    //Due to lack of overloading, we add dump methods directly to the objects in python:
    void dump_mat(const Material*m) { dump(m); }
    void dump_elem(const Element*m) { dump(m); }
    void dump_iso(const Isotope*m) { dump(m); }
    intptr_t py_mat_id(const Material*ptr) { return intptr_t(ptr); }

    void pyexport( py::module_ themod )
    {

      py::class_<Material,std::unique_ptr<Material, BlankDeleter<Material>>>(themod, "Material")
        .def("getName",&Material::getNameCStr)
        .def("density",&Material::density)
        .def("temperature",&Material::temperature)
        .def("pressure",&Material::pressure)
        .def("radiationLength",&Material::radiationLength)
        .def("nuclearInteractionLength",&Material::nuclearInteractionLength)
        .def("hasMeanExitationEnergy",&Material::hasMeanExitationEnergy)
        .def("meanExitationEnergy",&Material::meanExitationEnergy)
        .def("isSolid",&Material::isSolid)
        .def("isGas",&Material::isGas)
        .def("isLiquid",&Material::isLiquid)
        .def("isUndefinedState",&Material::isUndefinedState)
        .def("stateStr",&Material::stateCStr)
        .def("numberElements",&Material::numberElements)
        .def("elementFraction",&Material::elementFraction)
        .def("getElement",&Material::getElement,py::return_value_policy::reference)
        .def_property_readonly("elements", &py_get_elem_list)
        .def("dump",&dump_mat)
        .def("transient_id",&py_mat_id)//temporary workaround...
        ;

      py::class_<Element,std::unique_ptr<Element, BlankDeleter<Element>>>(themod, "Element")
        .def("getName",&Element::getNameCStr)
        .def("getSymbol",&Element::getSymbolCStr)
        .def("Z",&Element::Z)
        .def("N",&Element::N)
        .def("A",&Element::A)
        .def("naturalAbundances",&Element::naturalAbundances)
        .def("numberIsotopes",&Element::numberIsotopes)
        .def("isotopeRelativeAbundance",&Element::isotopeRelativeAbundance)
        .def("getIsotope",&Element::getIsotope,py::return_value_policy::reference)
        .def_property_readonly("isotopes", &py_get_isotope_list)
        .def("dump",&dump_elem)
        ;

      py::class_<Isotope,std::unique_ptr<Isotope, BlankDeleter<Isotope>>>(themod, "Isotope")
        .def("getName",&Isotope::getNameCStr)
        .def("Z",&Isotope::Z)
        .def("N",&Isotope::N)
        .def("A",&Isotope::A)
        .def("m",&Isotope::m)
        .def("dump",&dump_iso)
        ;
    }
  }

}
