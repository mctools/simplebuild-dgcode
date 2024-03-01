#include "Core/Python.hh"
#include "G4PhysicsLists/PhysListMgr.hh"

namespace {

  py::list pyPhysListMgr_getLists( bool with_descr,
                                   bool include_g4ref,
                                   bool include_custom )
  {
    py::list l;
    if ( !include_g4ref && !include_custom )
      return l;
    for ( auto& e : PhysListMgr::getAllLists() ) {
      bool is_custom = !e.pkg_name.empty();
      if ( !include_g4ref && !is_custom )
        continue;
      if ( !include_custom && is_custom )
        continue;
      if ( with_descr ) {
        std::string descr;
        if ( is_custom ) {
          descr = "List defined in package ";
          descr += e.pkg_name;
        } else {
          descr = "Geant4 reference list";
        }
        //FIXME: e.description could also be used?!?
        l.append( py::make_tuple(py::str(e.name),
                                 py::str(descr)) );
      } else {
        l.append( e.name );
      }
    }
    return l;
  }


}

PYTHON_MODULE( mod )
{
  mod.def("_get_lists",&pyPhysListMgr_getLists);
}

