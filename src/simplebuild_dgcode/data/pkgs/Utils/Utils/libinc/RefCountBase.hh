#ifndef Utils_RefCountBase_hh
#define Utils_RefCountBase_hh

#include <cassert>

namespace Utils {

  //FIXME: We should stop using this and use std::shared_ptr instead.

  //NOTE: Destructor of derived classes must not be public!

  class RefCountBase {
  public:
    unsigned refCount() const { return m_refCount; }
    void ref() const { ++m_refCount; }
    void unref() const
    {
      assert(m_refCount>0);
      if (m_refCount==1)
        delete this;
      else
        --m_refCount;
    }
    void unrefNoDelete() const
    {
      assert(m_refCount>0);
      --m_refCount;
    }
  protected:
    RefCountBase() : m_refCount(0) {}
    virtual ~RefCountBase(){}
  private:
    RefCountBase( const RefCountBase & );
    RefCountBase & operator= ( const RefCountBase & );
    mutable unsigned m_refCount;
  };

}

#endif
