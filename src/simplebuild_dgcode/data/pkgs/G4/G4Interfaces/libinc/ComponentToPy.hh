#ifndef G4Interfaces_ComponentToPy_hh
#define G4Interfaces_ComponentToPy_hh

#ifndef SBLDPY_MODNAME
#  error "The ComponentToPy.hh header can only be used in pymod_* folders."
#endif

#include "sbldpy/sbldpy.hh"
#include "G4Interfaces/ComponentHandles.hh"
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace G4Interfaces {

  //Helper returned by geoToPy, genToPy and filterToPy, which can be used to
  //add extra methods to the Python component objects. Usage:
  //
  //  SBLDPY_MODULE( _MyGeo )
  //  {
  //    G4Interfaces::geoToPy<MyGeo>( m )
  //      .def( "someMethod", &MyGeo::someMethod )
  //      .def( "otherMethod", &MyGeo::otherMethod, { "x", "y" } );
  //  }
  //
  //The methods must be member functions of the component class, and their
  //parameters and return values must be of types supported by sbldpy, except
  //that any integer type can be used (integer arguments are range checked), and
  //that parameters can be of type float.
  //Each method is provided as a function _method_<name> in the module, which
  //takes the handle of the component as first argument. The component_creator
  //function in G4Interfaces/python/ToPy.py adds these as methods to the
  //component objects.

  namespace detail {
    template<class A, class = void>
    struct ToPyParam {
      using type = A;
      static A conv( type a ) { return static_cast<A>( a ); }
    };
    template<class A>
    struct ToPyParam<A,std::enable_if_t<( std::is_integral_v<std::decay_t<A>>
                                          && !std::is_same_v<std::decay_t<A>,bool>
                                          && !std::is_same_v<std::decay_t<A>,std::int64_t> )>> {
      using U = std::decay_t<A>;
      using type = std::int64_t;
      static U conv( std::int64_t a )
      {
        if constexpr ( std::is_signed_v<U> ) {
          if ( a < std::numeric_limits<U>::min() || a > std::numeric_limits<U>::max() )
            throw std::out_of_range( "integer argument out of range" );
        } else {
          if ( a < 0 || static_cast<std::uint64_t>( a ) > std::numeric_limits<U>::max() )
            throw std::out_of_range( "integer argument out of range" );
        }
        return static_cast<U>( a );
      }
    };
    template<class A>
    struct ToPyParam<A,std::enable_if_t<std::is_same_v<std::decay_t<A>,float>>> {
      using type = double;
      static float conv( double a ) { return static_cast<float>( a ); }
    };
  }

  template <class T>
  class ComponentToPy {
  public:
    ComponentToPy( sbldpy::Module& mod, const char * what ) : m_m(mod), m_what(what) {}

    template<class R, class C, class... A>
    ComponentToPy& def( const char * name, R (C::*f)( A... ),
                        const sbldpy::Extra& e1 = sbldpy::Extra(), const sbldpy::Extra& e2 = sbldpy::Extra() )
    {
      static_assert( std::is_base_of_v<C,T>, "Method must be a member function of the component class" );
      const char * what = m_what;
      m_m.def( ( std::string( "_method_" ) + name ).c_str(),
               [f,what]( std::int64_t handle, typename detail::ToPyParam<A>::type... a ) -> R
               { return ( getComponentAs<T>( handle, what ).*f )( detail::ToPyParam<A>::conv( a )... ); },
               withHandle( e1 ), withHandle( e2 ) );
      return *this;
    }

    template<class R, class C, class... A>
    ComponentToPy& def( const char * name, R (C::*f)( A... ) const,
                        const sbldpy::Extra& e1 = sbldpy::Extra(), const sbldpy::Extra& e2 = sbldpy::Extra() )
    {
      static_assert( std::is_base_of_v<C,T>, "Method must be a member function of the component class" );
      const char * what = m_what;
      m_m.def( ( std::string( "_method_" ) + name ).c_str(),
               [f,what]( std::int64_t handle, typename detail::ToPyParam<A>::type... a ) -> R
               { return ( getComponentAs<T>( handle, what ).*f )( detail::ToPyParam<A>::conv( a )... ); },
               withHandle( e1 ), withHandle( e2 ) );
      return *this;
    }

  private:
    sbldpy::Module& m_m;
    const char * m_what;
    static sbldpy::Extra withHandle( sbldpy::Extra e )
    {
      if ( e.has_argnames )
        e.argnames.insert( e.argnames.begin(), "handle" );
      return e;
    }
  };

}

#endif
