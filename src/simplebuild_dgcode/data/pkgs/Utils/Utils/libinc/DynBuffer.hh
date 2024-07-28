#ifndef Utils_DynBuffer_hh
#define Utils_DynBuffer_hh

#include <cstring>
#include <cstddef>
#include <utility>
#include <cassert>
#include <type_traits>
#include <stdexcept>
#include <iterator>

namespace Utils {

  template<class TData>
  class DynBuffer final {

    //Simple dynamic buffer for POD's. Note that it was only really tested with
    //TData = char, and in particular the .resize_without_init(..) method might
    //actual call constructors in case of more complex non-builtin or compound
    //types. For that reason, we for simplicity assert !is_compound below, in
    //addition to the more usual POD traits:

    //Note that if the DynBuffer is at zero capacity, data(), begin(), and end()
    //all return nullptr.

    static_assert( std::is_trivially_destructible<TData>::value, "");
    static_assert( std::is_nothrow_destructible<TData>::value, "");
    static_assert( std::is_trivially_constructible<TData>::value, "");
    static_assert( std::is_nothrow_constructible<TData>::value, "");
    static_assert( std::is_standard_layout<TData>::value, "");
    static_assert( !std::is_compound<TData>::value, "");

  public:

    using size_t = std::size_t;
    using value_t = TData;

    DynBuffer() = default;

    DynBuffer( size_t initial_size )
    {
      if ( initial_size > 0 ) {
        TData * alloc_data = new TData[initial_size];
        if (!alloc_data)
          throw std::runtime_error("DynBuffer allocation failed");
        m_data = alloc_data;
        m_capacity = initial_size;
        m_dataEnd = m_data + initial_size;
        assert( size() == initial_size );
      } else {
        assert( size() == 0 );
        assert( empty() );
      }
    }

    ~DynBuffer()
    {
      delete[] m_data;
    }

    size_t size() const noexcept { return static_cast<size_t>(m_dataEnd
                                                              - m_data); };
    size_t capacity() const noexcept { return m_capacity; };
    bool empty() const noexcept { return m_data == m_dataEnd; };

    void swap( DynBuffer& o ) noexcept
    {
      std::swap( m_data, o.m_data );
      std::swap( m_dataEnd, o.m_dataEnd );
      std::swap( m_capacity, o.m_capacity );
    }

    //Move-only:
    DynBuffer( const DynBuffer& ) = delete;
    DynBuffer& operator=( const DynBuffer& ) = delete;

    DynBuffer( DynBuffer&& o )
    {
      this->swap( o );
      o->clear_and_dealloc();
    }

    DynBuffer& operator=( DynBuffer&& o )
    {
      this->swap( o );
      o->clear_and_dealloc();
      return *this;
    }

    value_t& operator[]( size_t n ) noexcept
    {
      assert( m_data && n < size() );
      return m_data[n];
    }

    const value_t& operator[]( size_t n ) const noexcept
    {
      assert( m_data && n < size() );
      return m_data[n];
    }

    void clear_and_dealloc()
    {
      if ( m_data ) {
        TData * detached_data = nullptr;
        std::swap(m_data,detached_data);
        m_dataEnd = nullptr;
        m_capacity = 0;
        delete[] detached_data;
      } else {
        assert( empty() && size() == 0 && m_capacity == 0 );
      }
    }

    void clear()
    {
      m_dataEnd = m_data;
    }

    void reserve( size_t n )
    {
      if ( n <= m_capacity )
        return;
      const size_t cur_size = size();
      assert( cur_size < n );
      DynBuffer o(n);
      if ( cur_size > 0 )
        std::memcpy( (void*)o.data(), (void*)m_data, sizeof(TData)*cur_size );
      o.m_dataEnd = o.m_data + cur_size;
      assert( o.size() < o.m_capacity );
      this->swap( o );
      o.clear_and_dealloc();//fixme just destructor
    }

    void resize_without_init( size_t n )
    {
      const size_t cur_size = size();
      if ( n == cur_size )
        return;
      if ( n > m_capacity )
        this->reserve(n);
      assert( n <= m_capacity );
      m_dataEnd = m_data + n;
    }

    TData * data() noexcept { return m_data; }
    const TData * data() const noexcept { return m_data; }
    TData * begin() noexcept { return m_data; }
    TData * end() noexcept { return m_dataEnd; }
    const TData * begin() const noexcept { return m_data; }
    const TData * end() const noexcept { return m_dataEnd; }

  private:
    TData * m_data = nullptr;
    TData * m_dataEnd = nullptr;
    size_t m_capacity = 0;
  };

}

#endif
