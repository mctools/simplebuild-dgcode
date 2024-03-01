#include "PluginUtils/PluginHelper.hh"
#include "NCrystal/internal/NCFileUtils.hh"
#include "NCrystal/internal/NCStrView.hh"
#include <cstdlib>
#include <fstream>

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

  //Non-templated version:
std::vector<PluginHelper::PluginInfo>
PluginHelper::getAvailablePluginsByType( std::string plugin_type,
                                         LoadDescriptions load_descr )
{
  //plugin_<plugintype>_<pluginkey>.txt
  static const std::string pattern0 = []()
  {
    static auto env_sbld_datadir = std::getenv( "SBLD_DATA_DIR" );
    if (!env_sbld_datadir)
      throw std::runtime_error("SBLD_DATA_DIR not set");
    return std::string(env_sbld_datadir) + "/*/plugin_";
  }();
  const std::string pattern_fn = std::string("plugin_")+plugin_type+"_";

  //construct pattern:
  std::string pattern = pattern0;
  pattern += plugin_type;
  pattern += "_*.txt";

  std::vector<PluginHelper::PluginInfo> res;
  std::vector<std::string> keys_seen; //{key,pkgname}
  for ( auto& filepath : NCrystal::ncglob( pattern ) ) {
    auto path_parts = NCrystal::StrView(filepath).split('/');
    if ( path_parts.size() < 3 )
      throw std::runtime_error("PluginHelper::getAvailablePluginsByType"
                               "unexpected error on path split");
    std::string pkgname = path_parts.at(path_parts.size()-2).to_string();
    auto& fn = path_parts.back();
    auto& fn_orig = path_parts.back();
    if (!fn.startswith(pattern_fn)||!fn.endswith(".txt"))
      throw std::runtime_error("PluginHelper::getAvailablePluginsByType"
                               "unexpected error on filename processing");
    fn = fn.substr(pattern_fn.size());
    std::string plugin_key = fn.substr(0,fn.size()-4).to_string();
    if (plugin_key.empty())
      throw std::runtime_error(std::string("Invalid name of plugin file: ")
                               +fn_orig.to_string());

    res.emplace_back();
    auto& p_info = res.back();
    p_info.plugin_key = plugin_key;
    keys_seen.push_back( plugin_key );
    p_info.pkgname = pkgname;
    if ( load_descr == LoadDescriptions::YES ) {
      std::ifstream f_in(filepath);
      std::stringstream f_buffer;
      f_buffer << f_in.rdbuf();
      p_info.description = NCrystal::trim2(f_buffer.str());
    }
  }
  std::sort( keys_seen.begin(), keys_seen.end() );
  if ( keys_seen.size() >= 2 ) {
    auto it = std::next( keys_seen.begin() );
    auto itE = keys_seen.end();
    for ( ; it!=itE; ++it )
      if ( *it == *std::prev(it) ) {
        std::ostringstream ss;
        ss << "Multiple "<<plugin_type<<" plugins provide the same"
           <<" plugin_key: \""<<*it<<"\".";
        throw std::runtime_error(ss.str());
      }
  }
  return res;

}
