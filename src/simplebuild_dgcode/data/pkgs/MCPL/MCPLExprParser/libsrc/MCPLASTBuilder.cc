#include "MCPLExprParser/MCPLASTBuilder.hh"
#include "MCPLDataExtractors.hh"
#include "ExprParser/ASTStdPhys.hh"
#include <cassert>

namespace MCPLExprParser {

  template<class TValue>
  class MCPLEEValBase : public ExprParser::ExprEntity<TValue> {
  public:
    MCPLEEValBase(const str_type name_, const mcpl_particle_t *& p)
      : ExprParser::ExprEntity<TValue>(), m_p(p), m_name(name_) {}
    virtual bool isConstant() const { return false; }
    virtual str_type name() const { return m_name; }
  protected:
    const mcpl_particle_t *& m_p;
    str_type m_name;
  };

  template<class TValue, TValue eval_func(const mcpl_particle_t*)>
  class MCPLEEVal final : public MCPLEEValBase<TValue> {
  public:
    typedef MCPLEEValBase<TValue> TSuper;
    using TSuper::TSuper;
    virtual TValue evaluate() const
    {
      assert(TSuper::m_p&&"did you remember to call MCPLASTBuilder::setCurrentParticle before evaluating the expression?");
      return eval_func(TSuper::m_p);
    }
  };

  template <int_type thefunc(const mcpl_particle_t*)>
  ExprEntityPtr wrap_extractor(const str_type& name, const mcpl_particle_t *& particleref)
  {
    return ExprParser::makeobj<MCPLEEVal<decltype(thefunc(0)),thefunc>>(name,particleref);
  }
  template <float_type thefunc(const mcpl_particle_t*)>
  ExprEntityPtr wrap_extractor(const str_type& name, const mcpl_particle_t *& particleref)
  {
    return ExprParser::makeobj<MCPLEEVal<decltype(thefunc(0)),thefunc>>(name,particleref);
  }

  ExprEntityPtr MCPLASTBuilder::createValue(const str_type& name) const
  {
    using ExprParser::makeobj;
    using ExprParser::create_constant;

    //standard math/physics constants:

    auto p = ASTBuilder::createValue(name);
    p = p ? p : ExprParser::create_standard_unit_or_constant(name);
    if (p)
      return p;

    //volatile value wrappers for data extracted from a mcpl_particle_t
    //instance. Variable names map to the correspondingly named function in
    //MCPLDataExtractors.hh:

    const mcpl_particle_t * & particleref = const_cast<MCPLASTBuilder*>(this)->m_currentParticle;

    //Evil but convenient macro (can't stringify with pure C++):
#   ifdef TESTRETURN
#     undef TESTRETURN
#   endif
#   define TESTRETURN(x) if (name==#x) { return wrap_extractor<DataExtractors::x>(name,particleref); }

    switch(name.empty()?'@':name[0]) {
    case 'd':
      TESTRETURN(dirx);
      TESTRETURN(diry);
      TESTRETURN(dirz); break;
    case 'e':
      TESTRETURN(ekin); break;
    case 'i':
      TESTRETURN(is_gamma);
      TESTRETURN(is_ion);
      TESTRETURN(is_neutrino);
      TESTRETURN(is_neutron);
      TESTRETURN(is_photon); break;
    case 'n':
      TESTRETURN(neutron_wl); break;
    case 'p':
      TESTRETURN(pdgcode);
      TESTRETURN(polx);
      TESTRETURN(poly);
      TESTRETURN(polz);
      TESTRETURN(posx);
      TESTRETURN(posy);
      TESTRETURN(posz); break;
    case 't':
      TESTRETURN(time); break;
    case 'u':
      TESTRETURN(userflags);
      TESTRETURN(ux);
      TESTRETURN(uy);
      TESTRETURN(uz); break;
    case 'w':
      TESTRETURN(weight); break;
    case 'x':
      TESTRETURN(x); break;
    case 'y':
      TESTRETURN(y); break;
    case 'z':
      TESTRETURN(z); break;
    }

    return 0;
  }

}
