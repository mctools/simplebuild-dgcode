#ifndef Utils_MemPool_hh
#define Utils_MemPool_hh

//A fast memory pool which does not allow individual chunks to be released,
//rather all chunks can be released at once.

#include <vector>

namespace Utils {

  template<unsigned NBytes,unsigned Count=1024>
  class MemPool {
  public:
    static const unsigned blocksize = Count;
    static const unsigned chunksize = NBytes;

    MemPool() { grow(); }
    ~MemPool() { releaseAll(); free(m_areas[0]); }
    char* acquire()
    {
      if (m_next == Count)
        grow();
      return &(m_current[ m_next++ * NBytes ]);
    }
    void releaseAll()
    {
      m_next = 0;
      assert(m_areas.size()>=1);
      auto it = m_areas.begin();
      auto itE = m_areas.end();
      ++it;//keep the first area for next time
      for(;it!=itE;++it)
	//        delete[] *it;
        free(*it);
      m_areas.resize(1);
      m_current = m_areas.front();
    }
  private:
    void grow()
    {
      m_next = 0;
      //m_areas.push_back(m_current = new char[NBytes*Count]);
      m_areas.push_back(m_current = (char*)malloc(NBytes*Count));
    }
    unsigned m_next;
    char * m_current;
    std::vector<char*> m_areas;
  };

}

#endif

