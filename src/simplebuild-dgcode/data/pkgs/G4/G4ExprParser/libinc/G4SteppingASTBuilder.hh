#ifndef G4ExprParser_G4SteppingASTBuilder_hh
#define G4ExprParser_G4SteppingASTBuilder_hh

#include "ExprParser/ASTBuilder.hh"

class G4Step;

namespace G4ExprParser {

  using ExprParser::ASTBuilder;
  using ExprParser::ExprEntityPtr;
  using ExprParser::float_type;
  using ExprParser::int_type;
  using ExprParser::str_type;

  class G4SteppingASTBuilder : public ASTBuilder {
  public:
    G4SteppingASTBuilder() : ASTBuilder(), m_currentStep(0) {};
    virtual ~G4SteppingASTBuilder(){}

    //Must always set current step here before attempting to evaluate expression
    //trees built with this class:
    void setCurrentStep(const G4Step* step) { m_currentStep = step; }

    //Better disallow copy/move/assign, because after copying previously created
    //expressions will still refer to m_currentStep in the original builder,
    //which might lead to surprises:
    G4SteppingASTBuilder & operator= ( const G4SteppingASTBuilder & ) = delete;
    G4SteppingASTBuilder & operator= ( G4SteppingASTBuilder && ) = delete;
    G4SteppingASTBuilder( const G4SteppingASTBuilder& ) = delete;
    G4SteppingASTBuilder( G4SteppingASTBuilder&& ) = delete;

  protected:
    virtual ExprEntityPtr createValue(const str_type& name) const;
    const G4Step * m_currentStep;
  };

}

#endif
