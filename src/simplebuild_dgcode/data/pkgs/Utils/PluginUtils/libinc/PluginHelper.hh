#ifndef dgcode_PluginHelper_hh
#define dgcode_PluginHelper_hh

#include "NCrystal/internal/NCDynLoader.hh"
#include <memory>
#include <string>

namespace PluginHelper {

  //Get the path to a package's shared library (empty if not found):
  std::string pkgLibPath( const std::string& packageName );

  //Load shared library into NCrystal::DynLoader object. If not found, return
  //nullptr (AllowAbsent::YES) or throw exception (AllowAbsent::NO):
  using DynLoaderPtr = std::shared_ptr<NCrystal::DynLoader>;
  enum class AllowAbsent { YES, NO };
  DynLoaderPtr loadPkgLib( const std::string& packageName,
                           AllowAbsent = AllowAbsent::NO );


}

#endif
