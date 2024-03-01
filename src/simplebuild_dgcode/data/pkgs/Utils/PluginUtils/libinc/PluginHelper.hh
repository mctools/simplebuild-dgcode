#ifndef dgcode_PluginHelper_hh
#define dgcode_PluginHelper_hh

#include "NCrystal/internal/NCDynLoader.hh"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <sstream>

namespace PluginHelper {

  //Get the path to a package's shared library (empty if not found):
  std::string pkgLibPath( const std::string& packageName );

  //Load shared library into NCrystal::DynLoader object. If not found, return
  //nullptr (AllowAbsent::YES) or throw exception (AllowAbsent::NO):
  using DynLoaderPtr = std::shared_ptr<NCrystal::DynLoader>;
  enum class AllowAbsent { YES, NO };
  DynLoaderPtr loadPkgLib( const std::string& packageName,
                           AllowAbsent = AllowAbsent::NO );

  //What follows are helpers for generic plugin mechanism, where packages
  //declare certain plugin factory functions via one or more text files in their
  //data/ directory. These factory functions must be have 'extern "C"' mangling,
  //and be present in the package's shared library.
  //
  //The name of the text file should be "plugin_<plugintype>_<pluginkey>.txt,
  //and it's contents can optionally be a short (possibly a one-liner)
  //description of the plugin. The factory function should have the named
  //"pluginfactory_<pkgname>_<plugintype>_<pluginkey>.txt", and a signature
  //which depends on <plugintype>. Note that <pluginkey> can contain
  //underscores, but plugintype can not!

  //Code wanting to support a given kind of plugin should define a struct with
  //fields defining the plugin, providing the <plugintype> name as well as the
  //plugin function signatures. Here is a silly example:
  //
  // struct PluginDef {
  //   using plugin_fct_signature_t = const SomeStuff*(const char*,bool);
  //   static constexpr const char * plugin_type = "somestuff";
  // }

  struct PluginInfo {
    std::string plugin_key;
    std::string pkgname;
    NCrystal::Optional<std::string> description;
  };

  enum class LoadDescriptions { YES, NO };

  template<class TPluginDef>
  std::vector<PluginInfo>
  getAvailablePlugins( LoadDescriptions = LoadDescriptions::NO );

  template<class TPluginDef>
  std::function<typename TPluginDef::plugin_fct_signature_t>
  loadPlugin( std::string plugin_key );

  //Non-templated version:
  std::vector<PluginInfo>
  getAvailablePluginsByType( std::string plugin_type,
                             LoadDescriptions = LoadDescriptions::NO );

}

#include "PluginUtils/PluginHelper.icc"

#endif
