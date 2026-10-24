#include "sbldpy/sbldpy.hh"
#include "G4PhysicsLists/PhysListMgr.hh"
#include <stdexcept>
#include <string>

namespace {

  //Returns one list per line, with the name and description separated by a
  //tab if with_descr is set (see G4PhysicsLists/python/__init__.py).
  std::string getLists( bool with_descr,
                        bool include_g4ref,
                        bool include_custom )
  {
    std::string res;
    if ( !include_g4ref && !include_custom )
      return res;
    auto append = [&res]( const std::string& s )
    {
      if ( s.find_first_of("\t\n") != std::string::npos )
        throw std::runtime_error("Unexpected tab or newline in physics list name or description");
      res += s;
    };
    for ( auto& e : PhysListMgr::getAllLists() ) {
      bool is_custom = !e.pkg_name.empty();
      if ( !include_g4ref && !is_custom )
        continue;
      if ( !include_custom && is_custom )
        continue;
      append( e.name );
      if ( with_descr ) {
        std::string descr;
        if ( is_custom ) {
          descr = "List defined in package ";
          descr += e.pkg_name;
        } else {
          descr = "Geant4 reference list";
        }
        //FIXME: e.description could also be used?!?
        res += '\t';
        append( descr );
      }
      res += '\n';
    }
    return res;
  }

}

SBLDPY_MODULE( _impl )
{
  m.def( "get_lists", &getLists, { "with_descr", "include_g4ref", "include_custom" } );
}
