#ifndef Utils_FastLookupSet_hh
#define Utils_FastLookupSet_hh

//Set built around a vector with insertion sorting.
//
//This is optimal in cases where the number of insertions is much lower than the
//number of queries.

#include <algorithm>
#include <vector>
#include <cassert>

namespace Utils {

  template<class T>
  class FastLookupSet {
  public:
    typedef typename std::vector<T>::size_type size_type;

    bool contains(const T&t) const
    {
      return std::binary_search(m_data.begin(),m_data.end(),t);
    }

    void insert(const T&t)
    {
      if (m_data.empty()||t>m_data.back()) {
        //fits nicely at the end
        m_data.push_back(t);
      } else {
        auto it = std::lower_bound(m_data.begin(), m_data.end(), t);
        assert(it!=m_data.end());//this was covered by the test against back() above!
        if (*it!=t)//*it==t means it is already contained
          m_data.insert(it, t);
      }
    }

    void reserve(size_type n) { m_data.reserve(n); }
    size_type capacity() const { return m_data.capacity(); }
    const T* begin() const { return &m_data[0]; }
    const T* end() const { return begin()+size(); }
    size_type size() const { return m_data.size(); }
    bool empty() const { return m_data.empty(); }
    size_type count(const T&t) const { return contains(t) ? 1 : 0; }

  private:
  typename std::vector<T> m_data;
  };

}

#endif
