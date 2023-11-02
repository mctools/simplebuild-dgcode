#ifndef ExprParser_ASTBuilder_hh
#define ExprParser_ASTBuilder_hh

#include "ExprParser/Tokenizer.hh"
#include "ExprParser/ASTNode.hh"

namespace ExprParser {

  //Builder interface for converting a list of tokens into an abstract syntax
  //tree. The default implementation parses the tokens using the shunting yard
  //algorithm, extended to not only handle litteral constants and binary infix
  //operators, but also unary pre/post-fix operators, parenthetical grouping,
  //named constants and functions of variadic or constant arity. Derived
  //implementations can override the protected virtual methods to provide custom
  //operators, constants or functions. The buildTree method would usually not be
  //overridden.

  class ASTBuilder {
  public:

    ASTBuilder(){}
    virtual ~ASTBuilder(){}

    virtual ExprEntityPtr buildTree(const TokenList&) const;

    //Convenience method going all the way from a user provided string to an
    //Evaluator class of the correct type (including bool) for the intended
    //usage:
    template<class TValue, void func_tokenCreator(const str_type&, TokenList&) = createTokens>
    Evaluator<TValue> createEvaluator(const str_type&, bool optimise = true ) const;

    //Forbid move/copy/assignment, to prevent nasty surprises in some derived
    //class use-cases:
    ASTBuilder & operator= ( const ASTBuilder & ) = delete;
    ASTBuilder & operator= ( ASTBuilder && ) = delete;
    ASTBuilder( const ASTBuilder& ) = delete;
    ASTBuilder( ASTBuilder&& ) = delete;

  protected:

    //Operators are implemented in classifyXXXOperator and createXXXOperator
    //sister functions, which must be implemented in a consistent manner.
    //
    //classifyXXXOperator methods must return operator precedence and (for
    //binary operators) their right/left associativity. Return value of 0
    //indicates that the identifier is not a known operator.
    //
    //The default implementation here provides standard mathematical symbolic
    //operators like in C++, with the addition that "^" and "**" means
    //exponentiation and "xor" means bitwise exclusive-or, and that python-like
    //"and", "or" and "not" can be used along with the C++ versions (&&, ||, !):

    virtual unsigned classifyPrefixOperator(const str_type& identifier) const;
    virtual unsigned classifyBinaryOperator(const str_type& identifier, bool& is_right_associative) const;
    virtual unsigned classifyPostfixOperator(const str_type& identifier) const;

    virtual ExprEntityPtr createPrefixOperator(const str_type& identifier, ExprEntityPtr arg) const;
    virtual ExprEntityPtr createBinaryOperator(const str_type& identifier, ExprEntityPtr arg1, ExprEntityPtr arg2) const;
    virtual ExprEntityPtr createPostfixOperator(const str_type& identifier, ExprEntityPtr arg) const;

    //Override to create named values (default contains a few standard mathematical constants):
    virtual ExprEntityPtr createValue(const str_type& name) const;

    //Override to provide functions (default implementation contains a few standard mathematical functions):
    virtual ExprEntityPtr createFunction(const str_type& name, ExprEntityList& args) const;

    //Default is to allow implied multiplication when a litteral number is
    //directly followed by a named value or function (as recognised by the
    //createValue/createFunction methods, and to do so with a precedence just
    //above that of explicit multiplication. Reimplement to tune or disable:
    virtual unsigned classifyImpliedMultiplication(str_type& symbol, bool& is_right_associative) const;

  private:
    struct Imp;
  };

}

#include "ASTBuilder.icc"

#endif
