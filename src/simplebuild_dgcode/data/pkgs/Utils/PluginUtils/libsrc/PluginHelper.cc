#include "PluginUtils/PluginHelper.hh"
#include "NCrystal/internal/NCFileUtils.hh"
#include <stdexcept>
#include <cstdlib>

std::string PluginHelper::pkgLibPath( const std::string& packageName )
{
  static const std::string pattern0 = []()
  {
    static auto env_sbld_libdir = std::getenv( "SBLD_LIB_DIR" );
    if (!env_sbld_libdir)
      throw std::runtime_error("SBLD_LIB_DIR not set");
    return std::string(env_sbld_libdir) + "/libPKG__";
  }();

  //construct pattern:
  std::string pattern = pattern0;
  pattern += packageName;
  pattern += ".*";

  auto libfiles = NCrystal::ncglob( pattern );
  if ( libfiles.empty() )
    return std::string();
  if ( libfiles.size() != 1 )
    throw std::runtime_error(std::string("unexpected error: multiple hits on ")
                             +pattern);
  return std::move( libfiles.front() );
}

PluginHelper::DynLoaderPtr PluginHelper::loadPkgLib( const std::string& pkg,
                                                     AllowAbsent allow_absent )
{
  static std::map<std::string,DynLoaderPtr> db;
  {
    auto it = db.find(pkg);
    if ( it != db.end() )
      return it->second;
  }
  {
    auto shared_lib_file = pkgLibPath( pkg );
    if ( shared_lib_file.empty() ) {
      if ( allow_absent == AllowAbsent::YES )
        return nullptr;
      throw std::runtime_error( std::string("Could not find shared library from package \"")
                                + pkg + std::string("\".") );
    }
    auto thelib = std::make_shared<NCrystal::DynLoader>( shared_lib_file );
    thelib->doNotClose();//to avoid problems with e.g. destructors run during
                         //shutdown of statics.
    db[pkg] = std::move(thelib);
  }
  {
    auto it = db.find(pkg);
    nc_assert_always( it != db.end() );
    return it->second;
  }
}
