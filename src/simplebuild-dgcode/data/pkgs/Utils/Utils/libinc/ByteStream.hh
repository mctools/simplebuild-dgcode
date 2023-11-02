#ifndef Utils_ByteStream_hh
#define Utils_ByteStream_hh

#include "Core/Types.hh"
#include <cassert>
#include <string>
#include <vector>
#include <cstring>

namespace ByteStream {

  //supports basic types (POD's) and std::strings with a length up to UINT16_MAX (65536).

  template<class T>
  inline T interpret(const char* data)
  {
    return *(reinterpret_cast<const T*>(data));
  }

  template<class T>
  inline void read(const char*& data, T&t)
  {
    t = interpret<T>(data);
    data += sizeof(T);
  }

  template<>
  inline void read(const char*& data, std::string&t)
  {
    std::uint16_t n;//nb, must cap at UINT16_MAX when writing
    read(data,n);
    t.assign(data,n);
    data += n;
  }

  template<>
  inline void read(const char *& data, std::vector<std::string>&t)
  {
    std::uint16_t n;//nb, must cap at UINT16_MAX when writing
    read(data,n);
    t.clear();
    t.resize(n);
    for (unsigned i=0;i<n;++i)
      read(data,t[i]);
  }

  template<class T>
  inline unsigned nbytesToWrite(const T&)
  {
    return sizeof(T);
  }

  template<>
  inline unsigned nbytesToWrite(const std::string&t)
  {
    return sizeof(std::uint16_t)+t.size();
  }

  template<class T>
  inline unsigned nbytesToWrite()
  {
    return sizeof(T);
  }

  template<>
  inline unsigned nbytesToWrite(const std::vector<std::string>&t)
  {
    assert(t.size()<UINT16_MAX);
    unsigned n = sizeof(std::uint16_t);//size
    auto itE=t.end();
    for (auto it=t.begin();it!=itE;++it)
      n+=nbytesToWrite(*it);
    return n;
  }

  template<class T>
  inline void write(char *& data, const T&t)
  {
    *(reinterpret_cast<T*>(data)) = t;
    data += sizeof(T);
  }

  template<>
  inline void write(char *& data, const std::string&t)
  {
    assert(t.size()<UINT16_MAX);
    write(data,std::uint16_t(t.size()));
    if (!t.empty()) {
      std::memcpy(data,&(t[0]),t.size());
      data+=t.size();
    }
  }

  template<>
  inline void write(char *& data, const std::vector<std::string>&t)
  {
    assert(t.size()<UINT16_MAX);
    write(data,std::uint16_t(t.size()));
    auto itE=t.end();
    for (auto it=t.begin();it!=itE;++it)
      write(data,*it);
  }



}


#endif
