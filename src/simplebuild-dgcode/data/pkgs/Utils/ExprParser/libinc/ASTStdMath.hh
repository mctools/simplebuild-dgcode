#ifndef ExprParser_ASTStdMath_hh
#define ExprParser_ASTStdMath_hh

#include "ExprParser/ASTNode.hh"

namespace ExprParser {

  //Helper functions used in the base ASTBuilder to provide standard
  //mathematical operators, constants and functions:

  ExprEntityPtr create_std_math_constant(const str_type& name);
  ExprEntityPtr create_std_math_function(const str_type& name, ExprEntityList& args);
  ExprEntityPtr create_std_math_prefix_operator(const str_type& name, ExprEntityPtr arg);
  ExprEntityPtr create_std_math_binary_operator(const str_type& name, ExprEntityPtr arg1, ExprEntityPtr arg2);
  bool classify_std_math_prefix_operator(const str_type& name, unsigned & precedence);
  bool classify_std_math_binary_operator(const str_type& name, unsigned & precedence, bool& is_right_associative);

  //Nodes representing standard operators and a few functions:
  ExprEntityPtr create_binaryaddition(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_unaryminus(ExprEntityPtr arg);
  ExprEntityPtr create_unaryplus(ExprEntityPtr arg);//does nothing except error on string input
  ExprEntityPtr create_binarysubtraction(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_binarymultiplication(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_binarydivision(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_binaryexponentiation(ExprEntityPtr arg1, ExprEntityPtr arg2);//args converted to floats
  ExprEntityPtr create_binaryexponentiation_integers(ExprEntityPtr arg1, ExprEntityPtr arg2);//pure int^int -> int
  ExprEntityPtr create_binarymodulo(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_booleannot(ExprEntityPtr arg);
  ExprEntityPtr create_booleanand(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_booleanor(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_cmp_equal(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_cmp_notequal(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_cmp_lt(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_cmp_gt(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_cmp_le(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_cmp_ge(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_bitwise_not(ExprEntityPtr arg);
  ExprEntityPtr create_bitwise_and(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_bitwise_or(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_bitwise_xor(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_bitwise_lshift(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_bitwise_rshift(ExprEntityPtr arg1, ExprEntityPtr arg2);
  ExprEntityPtr create_function_sum(ExprEntityList& args);
  ExprEntityPtr create_function_min(ExprEntityList& args);
  ExprEntityPtr create_function_max(ExprEntityList& args);
  ExprEntityPtr create_function_abs(ExprEntityPtr arg);
  ExprEntityPtr create_function_fabs(ExprEntityPtr arg);
  ExprEntityPtr create_inrange(ExprEntityPtr arg0,ExprEntityPtr arg1,ExprEntityPtr arg2);

}

#endif
