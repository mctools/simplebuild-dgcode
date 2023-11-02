#ifndef Utils_StringSort_hh
#define Utils_StringSort_hh

namespace Utils {

  struct fast_str_cmp {
    //Speed up maps/sets with string keys by sorting based on string length
    //before contents are considered.
    template<class Tstring>
    bool operator()(const Tstring& s1,const Tstring& s2) const
    {
      return s1.size()==s2.size() ? s1<s2 : s1.size()<s2.size();
    }
  };
}

#endif
