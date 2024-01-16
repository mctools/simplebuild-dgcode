#ifndef ExprParser_ASTDebug_hh
#define ExprParser_ASTDebug_hh

//Abstract syntax tree for expression evaluation.

#include "ExprParser/ASTNode.hh"
#include "ExprParser/ASTBuilder.hh"

namespace ExprParser {

  template<class type_arg>
  class ExprEntityVolatileValue : public ExprEntity<type_arg> {
    //Replacement for ConstantValue which declares itself non-const (probably
    //just useful for debugging AST).
  public:
    virtual str_type name() const { return str_type("VolatileValue_")+exprTypeChar<type_arg>(); }
    explicit ExprEntityVolatileValue(type_arg val) : ExprEntity<type_arg>(), m_val(val) {}
    virtual ~ExprEntityVolatileValue(){}
    virtual bool isConstant() const { return false; }
    virtual type_arg evaluate() const { return m_val; }
  private:
    type_arg m_val;
  };

  //If arg is constant, replace evaluation of it with a volatile value:
  ExprEntityPtr create_volatile(ExprEntityPtr arg);

  //Or directly create a volatile value:
  ExprEntityPtr create_volatile(float_type val);
  ExprEntityPtr create_volatile(int_type val);
  ExprEntityPtr create_volatile(const str_type& val);

  class ASTBuilderDbg : public ASTBuilder {
    //Adds a single function, "volatile(..)" which flags it's argument as
    //volatile for debugging purposes.
  protected:
    virtual ExprEntityPtr createFunction(const str_type& name, ExprEntityList& args) const;
  };

  str_type to_str(ExprEntityPtr p);
  void printTree(ExprEntityPtr p, const std::string& indent = "     | ",bool evaluate_volatile=true);//todo indent="" by default!
  void print_and_optimise_expr(ExprEntityPtr& p);//dumps tree, optimises (and possible replaces p) and prints tree again.

}


#endif
