#include "ExprParser/ASTBuilder.hh"
#include "ExprParser/ASTDebug.hh"
#include <cstdio>

int main(int argc,char** argv)
{
  namespace EP = ExprParser;
  std::string expression;
  for (int i = 1; i<argc;++i) {
    if (i>1)
      expression += " ";
    expression += argv[i];
  }

  try {
    EP::TokenList tokens;
    EP::createTokens(expression,tokens);
    auto p = EP::ASTBuilder().buildTree(tokens);
    switch (p->returnType()) {
    case EP::ET_INT: printf("%lli (integer)\n",(long long)EP::_eval<EP::int_type>(p)); break;
    case EP::ET_FLOAT: printf("%.16g (float)\n",EP::_eval<EP::float_type>(p)); break;
    case EP::ET_STRING: printf("\"%s\" (string)\n",EP::_eval<EP::str_type>(p).c_str()); break;
    }
  } catch (EP::InputError& e) {
    printf("%s : %s\n",e.epType(),e.epWhat());
  }
  return 0;

}
