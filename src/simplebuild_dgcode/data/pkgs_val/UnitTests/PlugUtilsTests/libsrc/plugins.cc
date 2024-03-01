#include <string>

int dummy_2423424265467()
{
  return 2;
}

extern "C" {
  const char * sbldplugindef_mytestplugintype_Bla_bla(const char* c,unsigned n)
  {
    std::string ss("Bla_bla::");
    for ( unsigned i = 0; i < n; ++i ) {
      ss+=c;
      ss+=',';
    }
    //Quick hack, since we can not return std::string from this (at least, it
    //causes a macos compilation issue).
    static std::string buf;
    buf = ss;
    return buf.c_str();
  }

  const char * sbldplugindef_mytestplugintype_WUHU(const char* c,unsigned n)
  {
    std::string ss("WUHU::");
    for ( unsigned i = 0; i < n*2; ++i ) {
      ss+=c;
      ss+=',';
    }
    static std::string buf;
    buf = ss;
    return buf.c_str();
  }

}
