#include "PluginUtils/PluginHelper.hh"
#include <iostream>

namespace {
  struct MyPluginDef {
    using plugin_fct_signature_t = const char *(const char*,unsigned);
    static constexpr const char * plugin_type = "mytestplugintype";
  };

}

int main() {
  for ( auto& e : PluginHelper::getAvailablePlugins<MyPluginDef>() ) {
    std::cout<<"Found "<<MyPluginDef::plugin_type<<" plugin in pkg "
             <<e.pkgname<<" : \""<<e.plugin_key<<"\""<<std::endl;
  }

  auto pfct1 = PluginHelper::loadPlugin<MyPluginDef>( "Bla_bla" );
  std::cout<<" pfct_Bla_bla(\"yiha\",3) = \""<<pfct1("yiha",3)<<"\""<<std::endl;
  std::cout<<" pfct_Bla_bla(\"muh\",2) = \""<<pfct1("muh",2)<<"\""<<std::endl;

  auto pfct2 = PluginHelper::loadPlugin<MyPluginDef>( "WUHU" );
  std::cout<<" pfct_WUHU(\"yiha\",3) = \""<<pfct2("yiha",3)<<"\""<<std::endl;
  std::cout<<" pfct_WUHU(\"muh\",2) = \""<<pfct2("muh",2)<<"\""<<std::endl;

  return 0;
}
