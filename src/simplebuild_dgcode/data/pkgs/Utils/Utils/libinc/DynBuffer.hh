#ifndef Utils_DynBuffer_hh
#define Utils_DynBuffer_hh

#include <cstring>
#include <utility>
#include <cassert>
#include <type_traits>
#include <stdexcept>

namespace Utils {

  template<class TData>
  class DynBuffer final {

    //Simple dynamic buffer for POD's. Note that it was only really tested with
    //TData = char, and in particular the .resize_without_init(..) method might
    //actual call constructors in case of more complex non-builtin or compound
    //types. For that reason, we for simplicity assert !is_compound below, in
    //addition to the more usual POD traits:

    static_assert( std::is_trivially_destructible<TData>::value, "");
    static_assert( std::is_nothrow_destructible<TData>::value, "");
    static_assert( std::is_trivially_constructible<TData>::value, "");
    static_assert( std::is_nothrow_constructible<TData>::value, "");
    static_assert( std::is_standard_layout<TData>::value, "");
    static_assert( !std::is_compound<TData>::value, "");

  public:

    using size_t = std::size_t;
    using value_t = TData;

    DynBuffer()
      : m_data(dataptr_noalloc())
    {
    }

    ~DynBuffer()
    {
      assert( bool(m_capacity) == bool(m_data != dataptr_noalloc()) );
      if ( m_capacity )
        delete[] m_data;
    }

    size_t size() const noexcept { return m_size; };
    size_t capacity() const noexcept { return m_capacity; };
    bool empty() const noexcept { return m_size == 0; };

    //Move-only:
    DynBuffer( const DynBuffer& ) = delete;
    DynBuffer& operator=( const DynBuffer& ) = delete;

    DynBuffer( DynBuffer&& o ) noexcept
    {
      std::swap( m_data, o.m_data );
      std::swap( m_size, o.m_size );
      std::swap( m_capacity, o.m_capacity );
    }

    DynBuffer& operator=( DynBuffer&& o ) noexcept
    {
      std::swap( m_data, o.m_data );
      std::swap( m_size, o.m_size );
      std::swap( m_capacity, o.m_capacity );
      return *this;
    }

    value_t& operator[]( size_t n ) noexcept
    {
      assert( m_data && n < m_size );
      return m_data[n];
    }

    const value_t& operator[]( size_t n ) const noexcept
    {
      assert( m_data && n < m_size );
      return m_data[n];
    }

    void clear_and_dealloc()
    {
      if ( m_capacity ) {
        TData * detached_data = dataptr_noalloc();
        std::swap( m_data, detached_data );
        m_size = 0;
        m_capacity = 0;
        assert( detached_data != dataptr_noalloc() );
        delete[] detached_data;
      }
    }

    void clear()
    {
      m_size = 0;
    }

    void reserve( size_t n )
    {
      if ( n <= m_capacity )
        return;
      TData * detached_data = new TData[n];//NB: If not a builtin type, this
                                           //actually initialises.
      if (!detached_data)
        throw std::runtime_error("DynBuffer allocation failed");
      assert( m_size < n );
      if ( m_size )
        std::memcpy( (void*)detached_data, (void*)m_data, sizeof(TData)*m_size );
      std::swap( m_data, detached_data );
      assert( bool(m_capacity) == bool(detached_data != dataptr_noalloc()) );
      if ( m_capacity )
        delete[] detached_data;
      m_capacity = n;
    }

    void resize_without_init( size_t n )
    {
      if ( n == m_size )
        return;
      if ( n < m_size ) {
        m_size = n;
        return;
      }
      if ( n > m_capacity )
        this->reserve(n);
      assert( n <= m_capacity );
      m_size = n;
    }

    TData * data() noexcept { return m_data; }
    const TData * data() const noexcept { return m_data; }
    TData * begin() noexcept { return m_data; }
    TData * end() noexcept { return m_data + m_size; }
    const TData * begin() const noexcept { return m_data; }
    const TData * end() const noexcept { return m_data + m_size; }

  private:
    static TData * dataptr_noalloc() noexcept
    {
      static TData dummy;
      return &dummy;
    }

    TData * m_data;
    size_t m_size = 0;
    size_t m_capacity = 0;
  };

}

#endif
