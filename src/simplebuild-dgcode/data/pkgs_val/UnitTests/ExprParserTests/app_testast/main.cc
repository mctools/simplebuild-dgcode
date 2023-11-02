#include "ExprParser/ASTBuilder.hh"
#include "ExprParser/ASTStdMath.hh"
#include "ExprParser/ASTDebug.hh"
#include <limits>

#define NVALS 17
ExprParser::ExprEntityPtr genvalue(int i) {
  using ExprParser::int_type;
  switch(i) {
    //only 1 volatile of each type, since it is the const values that plays a role in optimisation:
  case 0: return ExprParser::create_volatile((int_type)12l);
  case 1: return ExprParser::create_volatile(5.2);
  case 2: return ExprParser::create_volatile("Hello!");
    //integers:
  case 3: return ExprParser::create_constant((int_type)0l);
  case 4: return ExprParser::create_constant((int_type)-1l);
  case 5: return ExprParser::create_constant((int_type)1l);
  case 6: return ExprParser::create_constant((int_type)-2014l);
  case 7: return ExprParser::create_constant((int_type)5l);
    //TODO: add negative value < -1?
    //floating points:
  case 8: return ExprParser::create_constant(0.0);
  case 9: return ExprParser::create_constant(1.0);
  case 10: return ExprParser::create_constant(-1.0);
  case 11: return ExprParser::create_constant(std::numeric_limits<ExprParser::float_type>::infinity());
  case 12: return ExprParser::create_constant(-std::numeric_limits<ExprParser::float_type>::infinity());
  case 13: return ExprParser::create_constant(-0.123);
  case 14: return ExprParser::create_constant(17.0);
    //TODO: add nan? and negative float < -1.0?
    //strings:
  case 15: return ExprParser::create_constant("there");
  case 16: return ExprParser::create_constant("");
  default:
    throw std::runtime_error("genvalue idx out of range");
  }
  return 0;
}

int main(int,char**) {

  constexpr const char * divider = "---------------------------------\n";
  for (int i1 = 0; i1 < NVALS; ++i1)
    for (int i2 = 0; i2 < NVALS; ++i2) {
      printf(divider);
      auto v1 = genvalue(i1);
      auto v2 = genvalue(i2);
      ExprParser::ExprEntityPtr expr;
      try {
        expr = ExprParser::create_binaryaddition(v1,v2);
      } catch (const std::runtime_error& error) {
        auto s1 = to_str(v1);
        auto s2 = to_str(v2);
        printf("ExprParser::create_binaryaddition(%s,%s) gives exception\n",s1.c_str(),s2.c_str());
        expr=0;
      }
      print_and_optimise_expr(expr);
      try {
        expr = ExprParser::create_binarysubtraction(v1,v2);
      } catch (const std::runtime_error& error) {
        auto s1 = to_str(v1);
        auto s2 = to_str(v2);
        printf("ExprParser::create_binarysubtraction(%s,%s) gives exception\n",s1.c_str(),s2.c_str());
        expr=0;
      }
      print_and_optimise_expr(expr);
      try {
        expr = ExprParser::create_binarymultiplication(v1,v2);
      } catch (const std::runtime_error& error) {
        auto s1 = to_str(v1);
        auto s2 = to_str(v2);
        printf("ExprParser::create_binarymultiplication(%s,%s) gives exception\n",s1.c_str(),s2.c_str());
        expr=0;
      }
      print_and_optimise_expr(expr);
      bool v2iszero = ( (v2->returnType()==ExprParser::ET_INT && ExprParser::_eval<ExprParser::int_type>(v2)==0)
                        || (v2->returnType()==ExprParser::ET_FLOAT && ExprParser::_eval<ExprParser::float_type>(v2)==0.0) );
      if (!v2iszero) {
        try {
          expr = ExprParser::create_binarydivision(v1,v2);
        } catch (const std::runtime_error& error) {
          auto s1 = to_str(v1);
          auto s2 = to_str(v2);
          printf("ExprParser::create_binarydivision(%s,%s) gives exception\n",s1.c_str(),s2.c_str());
          expr=0;
        }
        print_and_optimise_expr(expr);
      }
      if (i2==0) {
        try {
          ExprParser::ExprEntityList args = {v1};
          expr = ExprParser::create_std_math_function("sin",args);
            //create_create_std_math_functionunary_function_f2f<std::sin>(v1,"sin");
        } catch (const std::runtime_error& error) {
          auto s1 = to_str(v1);
          printf("ExprParser::create_std_math_function(\"sin\", {%s}) gives exception\n",s1.c_str());
          expr=0;
        }
        print_and_optimise_expr(expr);
      }
      try {
        expr = ExprParser::create_booleanand(v1,v2);
      } catch (const std::runtime_error& error) {
        auto s1 = to_str(v1);
        auto s2 = to_str(v2);
        printf("ExprParser::create_booleanand(%s,%s) gives exception\n",s1.c_str(),s2.c_str());
        expr=0;
      }
      print_and_optimise_expr(expr);
      try {
        expr = ExprParser::create_booleanor(v1,v2);
      } catch (const std::runtime_error& error) {
        auto s1 = to_str(v1);
        auto s2 = to_str(v2);
        printf("ExprParser::create_booleanor(%s,%s) gives exception\n",s1.c_str(),s2.c_str());
        expr=0;
      }
      print_and_optimise_expr(expr);
    }

  for (int i1 = 0; i1 < NVALS; ++i1) {
    printf(divider);
    auto v = genvalue(i1);
    ExprParser::ExprEntityPtr expr;
    try {
      expr = ExprParser::create_unaryminus(v);
    } catch (const std::runtime_error& error) {
      auto s = to_str(v);
      printf("ExprParser::create_unaryminus(%s) gives exception\n",s.c_str());
      expr=0;
    }
    print_and_optimise_expr(expr);
    try {
      expr = ExprParser::create_unarybool(v);
    } catch (const std::runtime_error& error) {
      auto s = to_str(v);
      printf("ExprParser::create_unarybool(%s) gives exception\n",s.c_str());
      expr=0;
    }
    print_and_optimise_expr(expr);
    try {
      expr = ExprParser::create_booleannot(v);
    } catch (const std::runtime_error& error) {
      auto s = to_str(v);
      printf("ExprParser::create_booleannot(%s) gives exception\n",s.c_str());
      expr=0;
    }
    print_and_optimise_expr(expr);
  }


  return 0;
}

