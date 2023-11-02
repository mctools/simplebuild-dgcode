#ifndef Utils_DelayedAllocVector_hh
#define Utils_DelayedAllocVector_hh

#include <memory>//unique_ptr
#include <vector>
#include <cassert>
#include <algorithm>//std::swap before c++11
#include <utility>//std::swap after c++11
#include <stdexcept>

//Fixed-length vector with on-demand storage behind the scenes. Blocksize should
//be a power of 2 for highest efficiency.

namespace Utils {

  template <class TStorage = double, size_t BLOCKSIZE = 64>
  class DelayedAllocVector {
  public:

    typedef TStorage value_type;
    static const size_t block_size = BLOCKSIZE;

    //Initialise vector of length "size" with all elements initialised to zero:
    DelayedAllocVector(size_t size = 0);
    ~DelayedAllocVector();

    //Interface similar to std::vector:
    size_t size() const { return m_size; }
    void clear();
    void resize(size_t);
    TStorage operator[](size_t) const;//never allocates
    TStorage& operator[](size_t);//might perform an allocation behind the scenes.
    void swap(DelayedAllocVector& other);

    //Reset all cell content to zero:
    void reset();

    //Advanced usage: Access blocks directly (might be used to implement efficient
    //I/O and persistency schemes):
    typedef std::vector<TStorage> TBlockData;
    size_t blockCount() const { return m_data.size(); }
    size_t nextAllocBlock(size_t iblock_lower ) const;//get idx of first allocated block >= iblock_lower (blockCount if None)
    TBlockData * block(size_t iblock) { assert(iblock<m_data.size()); return m_data[iblock].get(); }

    //Add contents from "other" to this instance, and leave "other" reset afterwards.
    void merge(DelayedAllocVector& other);

    //Move construction and assignment (leaves other as size 0 vector):
    DelayedAllocVector(DelayedAllocVector&& other);
    DelayedAllocVector& operator=(DelayedAllocVector&& other);

  private:
    //Forbid copy/assignment:
    DelayedAllocVector( const DelayedAllocVector & );
    DelayedAllocVector & operator= ( const DelayedAllocVector & );
    std::vector<std::unique_ptr<TBlockData> > m_data;
    size_t m_size;
  };

  ////////////////////////////
  // Inline implementations //
  ////////////////////////////

  template <class TStorage, size_t BLOCKSIZE>
  inline void DelayedAllocVector<TStorage,BLOCKSIZE>::clear()
  {
    m_data.clear();
    m_size = 0;
  }

  template <class TStorage, size_t BLOCKSIZE>
  inline void DelayedAllocVector<TStorage,BLOCKSIZE>::swap(DelayedAllocVector& other)
  {
    std::swap(m_size, other.m_size);
    std::swap(m_data, other.m_data);
  }

  //Move constructor (leaves other as size 0 vector):
  template <class TStorage, size_t BLOCKSIZE>
  inline DelayedAllocVector<TStorage,BLOCKSIZE>::DelayedAllocVector(DelayedAllocVector&& other)
    : m_size(other.m_size)
  {
    std::swap(m_data,other.m_data);
    other.m_size = 0;
  }

  template <class TStorage, size_t BLOCKSIZE>
  inline DelayedAllocVector<TStorage,BLOCKSIZE>& DelayedAllocVector<TStorage,BLOCKSIZE>::operator=(DelayedAllocVector<TStorage,BLOCKSIZE>&& other)
  {
    //Destroy visible resources:
    m_data.clear();
    m_size = 0;
    //(Move) assign all members:
    m_data = std::move(other.m_data);
    m_size = other.m_size;
    //Make rhs resource-less:
    other.m_size = 0;
    assert(other.m_data.empty());
  }

  template <class TStorage, size_t BLOCKSIZE>
  inline void DelayedAllocVector<TStorage,BLOCKSIZE>::resize(size_t ss)
  {
    if (m_size)
      throw std::runtime_error("DelayedAllocVector::resize not implemented yet for non-empty vectors");
    m_size = ss;
    m_data.resize(ss / BLOCKSIZE + (ss%BLOCKSIZE?1:0));
  }

  template <class TStorage, size_t BLOCKSIZE>
  inline size_t DelayedAllocVector<TStorage,BLOCKSIZE>::nextAllocBlock(size_t iblock_lower ) const {
    size_t nb = m_data.size();
    while (iblock_lower < nb) {
      if (m_data[iblock_lower].get())
        return iblock_lower;
      ++iblock_lower;
    }
    return nb;
  }

  template <class TStorage, size_t BLOCKSIZE>
  inline DelayedAllocVector<TStorage,BLOCKSIZE>::DelayedAllocVector(size_t ssize)
    : m_size(ssize)
  {
    m_data.resize(ssize / BLOCKSIZE + (ssize%BLOCKSIZE?1:0));
  }

  template <class TStorage, size_t BLOCKSIZE>
  DelayedAllocVector<TStorage,BLOCKSIZE>::~DelayedAllocVector()
  {
  }

  template <class TStorage, size_t BLOCKSIZE>
  TStorage DelayedAllocVector<TStorage,BLOCKSIZE>::operator[](size_t idx) const
  {
    //this is slow in gcc 4.9.2 dbg builds due to non-inlining of unique_ptr methods:
    //  auto& blockdata = m_data[idx / BLOCKSIZE];
    //  return blockdata ? (*blockdata)[idx%BLOCKSIZE] : 0;
    //So we do:
    TBlockData * ptr = m_data[idx / BLOCKSIZE].get();
    return ptr ? (*ptr)[idx%BLOCKSIZE] : 0;
  }

  template <class TStorage, size_t BLOCKSIZE>
  TStorage& DelayedAllocVector<TStorage,BLOCKSIZE>::operator[](size_t idx)
  {
    assert(idx<m_size);
    size_t iblock = idx / BLOCKSIZE;
    assert(iblock < m_data.size());
    auto& blockdata = m_data[iblock];
    TBlockData * ptr = blockdata.get();//work on raw pointers to avoid slow
    //dbg builds (at least in gcc 4.9.2):
    if (!ptr) {
      size_t alloc_size = (iblock+1 == m_data.size() && m_size % BLOCKSIZE)
        ? m_size % BLOCKSIZE : BLOCKSIZE;
      assert(alloc_size==BLOCKSIZE||iblock+1 == m_data.size());
      assert(alloc_size>0&&alloc_size<=BLOCKSIZE);
      blockdata = std::unique_ptr<TBlockData>( ptr = new TBlockData( alloc_size, 0.0  ));
    }
    return (*ptr)[idx%BLOCKSIZE];
  }

  template <class TStorage, size_t BLOCKSIZE>
  void DelayedAllocVector<TStorage,BLOCKSIZE>::reset()
  {
    for (auto it = m_data.begin(); it!=m_data.end(); ++it)
      it->reset();
  }

  template <class TStorage, size_t BLOCKSIZE>
  void DelayedAllocVector<TStorage,BLOCKSIZE>::merge(DelayedAllocVector& other)
  {
    if (m_size!=other.m_size)
      throw std::runtime_error("can only merge DelayedAllocVector's vectors of equal size\n");

    auto itOB = other.m_data.begin();
    auto itOE = other.m_data.end();
    for (auto itO = itOB; itO!=itOE; ++itO) {
      TBlockData* optr = itO->get();
      if (!optr)
        continue;
      assert((size_t)(itO-itOB)<m_data.size());
      auto& unique_ptr = m_data[itO-itOB];
      TBlockData* ptr = unique_ptr.get();
      if (ptr) {
        assert(ptr->size()==optr->size());
        auto itElem = ptr->begin();
        for (auto itElemO = optr->begin(); itElemO != optr->end(); ++itElemO, ++itElem)
          *itElem += *itElemO;
        itO->reset();
      } else {
        unique_ptr = std::move(*itO);
        assert(itO->get()==0);
      }
    }
  }
}

namespace std {
  template<>
  inline void swap(Utils::DelayedAllocVector<>& a,Utils::DelayedAllocVector<>& b) {
    a.swap(b);
  }
}
#endif
