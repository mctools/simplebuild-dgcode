#include "G4PhysicsLists/PhysListMgr.hh"
#include "NCrystal/internal/NCStrView.hh"
#include <cstdlib>
#include "G4PhysListFactory.hh"
#include "PluginUtils/PluginHelper.hh"

namespace PhysListMgr {
  namespace {
    struct dgcode_PluginDef_G4PL {
      using plugin_fct_signature_t = G4VUserPhysicsList *();
      static constexpr const char * plugin_type = "g4physlist";
    };

    G4VUserPhysicsList *
    createReferencePhysicsList(const std::string& name)
    {
      //G4 provided reference lists:
      G4PhysListFactory reflist_factory;
      return reflist_factory.GetReferencePhysList(name);
    }

    std::vector<std::string> getAllReferenceListNames()
    {
      //G4 provided reference lists:
      G4PhysListFactory reflist_factory;
      auto refpl_buggy = reflist_factory.AvailablePhysLists();
      //alternative endings for EM physics:
      auto refplEM = reflist_factory.AvailablePhysListsEM();

      //Workaround bug in G4PhysListFactory: QGSP is too much, ShieldingLEND is
      //missing. Also, ShieldingLEND should only be there when G4LENDDATA is
      //set.
      static const std::string envG4LENDDATA = []() {
        const char * envG4LENDDATA_cstr = std::getenv("G4LENDDATA");
        return std::string( envG4LENDDATA_cstr ? envG4LENDDATA_cstr : "" );
      }();

      bool allowLEND = !envG4LENDDATA.empty();

      std::vector<std::string> refpl;
      bool sawShieldingLEND = false;
      for (auto it = refpl_buggy.begin();it != refpl_buggy.end();++it) {
        if (*it=="QGSP")
          continue;
        if (*it=="ShieldingLEND") {
          sawShieldingLEND = true;
          if (allowLEND)
            refpl.push_back(*it);
        } else {
          refpl.push_back(*it);
        }
      }
      if (allowLEND&&!sawShieldingLEND)
        refpl.push_back("ShieldingLEND");

      //combine hadronic and em parts (a large number admittedly):
      std::vector<std::string> v;
      v.reserve(128);
      for (auto it = refpl.begin();it != refpl.end();++it) {
        for (auto itEM = refplEM.begin();itEM!= refplEM.end();++itEM)
          v.push_back(*it+*itEM);
      }
      return v;
    }
  }
}

std::vector<PhysListMgr::PhysListInfo>
PhysListMgr::getAllLists( LoadDescriptions load_descr_enum )
{
  bool load_descr( load_descr_enum==LoadDescriptions::YES );
  auto load_descr_ph( load_descr
                      ? PluginHelper::LoadDescriptions::YES
                      : PluginHelper::LoadDescriptions::NO );
  std::vector<PhysListMgr::PhysListInfo> res;

  //First the G4 ref lists:
  const std::string g4reflistdescr("Geant4 reference list");
  for ( auto& reflistname : getAllReferenceListNames() ) {
    if ( NCrystal::StrView( reflistname ).startswith("PL_") )
      throw std::runtime_error("g4 ref lists are assumed"
                               " to never start with \"PL_\"");
    res.emplace_back();
    auto& e = res.back();
    e.name = reflistname;
    if ( load_descr )
      e.description = g4reflistdescr;
  }
  //Then the custom plugins:

  for ( auto & plugininfo : PluginHelper::getAvailablePlugins
          <dgcode_PluginDef_G4PL>( load_descr_ph ) )
    {
      res.emplace_back();
      auto& e = res.back();
      e.name = std::string("PL_")+plugininfo.plugin_key;
      e.pkg_name = plugininfo.pkgname;
      if ( plugininfo.description.has_value() )
        e.description = plugininfo.description.value();
    }
  return res;
}

G4VUserPhysicsList * PhysListMgr::createList( std::string name )
{
  NCrystal::StrView name_sv(name);
  if ( name_sv.startswith("PL_") ) {
    //A custom plugins:
    name_sv = name_sv.substr(3);//discard "PL_"
    auto pl_fct
      = PluginHelper::loadPlugin<dgcode_PluginDef_G4PL>(name_sv.to_string());
    return pl_fct();
  } else {
    return createReferencePhysicsList( name );
  }
}
