#ifndef MCPLExprParser_MCPLASTBuilder_hh
#define MCPLExprParser_MCPLASTBuilder_hh

#include "ExprParser/ASTBuilder.hh"
#include "MCPL/mcpl.h"

namespace MCPLExprParser {

  using ExprParser::ASTBuilder;
  using ExprParser::ExprEntityPtr;
  using ExprParser::float_type;
  using ExprParser::int_type;
  using ExprParser::str_type;

  class MCPLASTBuilder : public ASTBuilder {
  public:
    MCPLASTBuilder() : ASTBuilder(), m_currentParticle(0) {};
    virtual ~MCPLASTBuilder(){}

    //Must always set current particle here before attempting to evaluate expression
    //trees built with this class:
    void setCurrentParticle(const mcpl_particle_t * p ) { m_currentParticle = p; }

    //Better disallow copy/move/assign, because after copying previously created
    //expressions will still refer to m_currentParticle in the original builder,
    //which might lead to surprises:
    MCPLASTBuilder & operator= ( const MCPLASTBuilder & ) = delete;
    MCPLASTBuilder & operator= ( MCPLASTBuilder && ) = delete;
    MCPLASTBuilder( const MCPLASTBuilder& ) = delete;
    MCPLASTBuilder( MCPLASTBuilder&& ) = delete;

  protected:
    virtual ExprEntityPtr createValue(const str_type& name) const;
    const mcpl_particle_t * m_currentParticle;
  };


}

#endif
