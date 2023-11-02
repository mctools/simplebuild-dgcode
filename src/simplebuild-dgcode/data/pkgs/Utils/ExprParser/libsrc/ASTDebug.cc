#include "ExprParser/ASTDebug.hh"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace ExprParser {

  ExprEntityPtr create_volatile(ExprEntityPtr arg)
  {
    if (!arg->isConstant())
      return arg;
    switch(arg->returnType()) {
    case ET_FLOAT: return makeobj<ExprEntityVolatileValue<float_type>>(_eval<float_type>(arg));
    case ET_INT: return makeobj<ExprEntityVolatileValue<int_type>>(_eval<int_type>(arg));
    case ET_STRING: return makeobj<ExprEntityVolatileValue<str_type>>(_eval<str_type>(arg));
    }
    assert_logic(false);
    return 0;
  }

  ExprEntityPtr create_volatile(float_type val) { return makeobj<ExprEntityVolatileValue<float_type>>(val); }
  ExprEntityPtr create_volatile(int_type val) { return makeobj<ExprEntityVolatileValue<int_type>>(val); }
  ExprEntityPtr create_volatile(const str_type& val) { return makeobj<ExprEntityVolatileValue<str_type>>(val); }

  ExprEntityPtr ASTBuilderDbg::createFunction(const str_type& name, ExprEntityList& args) const
  {
    if (name=="volatile") {
      if (args.size()!=1)
        EXPRPARSER_THROW(ParseError,"wrong number of arguments in call to function volatile");
      return create_volatile(args.front());
    }
    return ASTBuilder::createFunction(name,args);
  }

  str_type to_str(ExprEntityPtr p)
  {
    std::ostringstream s;
    if (p->returnType()==ExprParser::ET_INT) {
      s << ExprParser::_eval<ExprParser::int_type>(p);
    } else if (p->returnType()==ExprParser::ET_FLOAT) {
      float_type v = ExprParser::_eval<ExprParser::float_type>(p);
      if (std::isnan(v))
        s << "nan";//prevent signed nan printing like "-nan" on some platforms and breaking unit tests
      else
        s << std::fixed << std::setprecision(4) << v;
    } else {
      s << '"' << ExprParser::_eval<ExprParser::str_type>(p) << '"';
    }
    return s.str();
  }

  void printTree(ExprEntityPtr p, const std::string& indent, bool evaluate_volatile)
  {
    if (evaluate_volatile||p->isConstant()) {
      std::string res = to_str(p);
      printf("%s%s(%s)\n",indent.c_str(),p->name().c_str(),res.c_str());
    } else {
      printf("%s%s\n",indent.c_str(),p->name().c_str());
    }
    auto& children = p->children();
    for(auto& c : children) {
      printTree(c,indent+"  ",evaluate_volatile);
    }
  }

  void print_and_optimise_expr(ExprEntityPtr& p)
  {
    if (!p)
      return;
    printf("  AST:\n");
    printTree(p);
    bool changed(false);
    if (p->returnType()==ExprParser::ET_INT) {
      auto v = ExprParser::_eval<ExprParser::int_type>(p);
      optimise(p);
      changed = (v!=ExprParser::_eval<ExprParser::int_type>(p));
    }
    else if (p->returnType()==ExprParser::ET_FLOAT) {
      auto v = ExprParser::_eval<ExprParser::float_type>(p);
      optimise(p);
      auto vo = ExprParser::_eval<ExprParser::float_type>(p);
      changed = ! ( (std::isnan(v) && std::isnan(vo)) || v == vo );
    }
    else {
      assert_logic(p->returnType()==ExprParser::ET_STRING);
      auto v = ExprParser::_eval<ExprParser::str_type>(p);
      optimise(p);
      changed = (v!=ExprParser::_eval<ExprParser::str_type>(p));
    }
    printf("  Optimised AST:\n");
    printTree(p);
    if (changed)
      EXPRPARSER_THROW(LogicError,"optimisation changed value");
  }


}
