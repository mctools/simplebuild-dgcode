#include "ExprParser/Tokenizer.hh"//todo: rename ?
#include "ExprParser/ASTDebug.hh"
#include <cmath>

namespace EP = ExprParser;

EP::ExprEntityPtr _testexpr(const std::string& s)
{
  printf("--------------------------------------------------\n");
  printf("Testing expression \"%s\"\n",s.c_str());
  EP::TokenList tk;
  EP::ExprEntityPtr e;
  try {
    EP::createTokens(s,tk);
  } catch (EP::InputError& error) {
    printf("createTokens(\"%s\") gives exception: %s : %s\n",s.c_str(),error.epType(),error.epWhat());
    return e;
  }
  std::string msg;
  msg.reserve(128);
  try {
    e = EP::ASTBuilderDbg().buildTree(tk);
  } catch (EP::InputError& error) {
    printf("ASTBuilder.buildTree(..) gives exception: %s : %s\n",error.epType(),error.epWhat());
    e=0;
  }
  try {
    if (e)
      to_str(e);
  } catch (EP::InputError& error) {
    printf("Resulting AST gives exception on evaluation: %s : %s\n",error.epType(),error.epWhat());
    e=0;
  }

  print_and_optimise_expr(e);
  return e;
}


bool floatCompatible(const double& val1, const double& val2)
{
  const double eps = 1.0e-10;
  return (fabs(val1-val2)/(1.0+std::max(fabs(val1),fabs(val2)))<eps) || ((val1!=val1)&&(val2!=val2));
}

void testexpr(const std::string& s)
{
  //expression building expected to fail
  auto e = _testexpr(s);
  if (e)
    throw std::runtime_error("expression test failed");
}

void testexpr(const std::string& s, EP::float_type res)
{
  //expression building expected to produce float with value of res
  auto e = _testexpr(s);
  if (!e||e->returnType()!=EP::ET_FLOAT||!floatCompatible(EP::_eval<EP::float_type>(e),res))
    throw std::runtime_error("expression test failed");
}

void testexpr(const std::string& s, EP::int_type res)
{
  //expression building expected to produce int with value of res
  auto e = _testexpr(s);
  if (!e||e->returnType()!=EP::ET_INT||EP::_eval<EP::int_type>(e)!=res)
    throw std::runtime_error("expression test failed");
}

void testexpr(const std::string& s, int res)
{
  //overload needed to avoid specifying integeres with 'l' postfix.
  testexpr(s,(EP::int_type)res);
}

void testexpr(const std::string& s, const EP::str_type& res)
{
  //expression building expected to produce string with value of res
  auto e = _testexpr(s);
  if (!e||e->returnType()!=EP::ET_STRING||EP::_eval<EP::str_type>(e)!=res)
    throw std::runtime_error("expression test failed");
}

int main(int,char**) {
  testexpr("1+2.0",3.0);
  testexpr("1+2",3);
  testexpr("(1+2.0)",3.0);
  testexpr("(1+2.0");
  testexpr("1+2.0)");
  testexpr("1+2.0+3",6.0);
  testexpr("1+2.0*3",7.0);
  testexpr("(2.0*3)+1",7.0);
  testexpr("2.0*3+1",7.0);
  testexpr("2.0*3+-1",5.0);
  testexpr("2.0*3+-+--1",5.0);
  testexpr("2.0*-3",-6.0);
  testexpr("2*-1",-2);
  testexpr("sin(2.0)",0.90929742682568170942);
  testexpr("asin(sin(0.513))",0.513);
  testexpr("acos(cos(0.513))",0.513);
  testexpr("atan(tan(0.513))",0.513);
  testexpr("asinh(sinh(0.513))",0.513);
  testexpr("acosh(cosh(0.513))",0.513);
  testexpr("atanh(tanh(0.513))",0.513);
  testexpr("erf(1)",0.84270079294971500516453);
  testexpr("erfc(1.)",0.15729920705028499483547);
  testexpr("sqrt(4.0)",2.0);
  testexpr("cbrt(27)",3.0);
  testexpr("expm1(1)",1.7182818284590450907956);
  testexpr("exp(10.0)",22026.465794806717894971);
  testexpr("exp2(3.4)",10.556063286183153593356);

  testexpr("sum(5.5)",5.5);
  testexpr("sum()",0);
  testexpr("sum(1,2)",3);
  testexpr("sum(2.2,1.0,-1.3)",1.9);
  testexpr("sum(2.2,1,-1.3)",1.9);
  testexpr("sum(2,1,-1,17,-18)",1);
  testexpr("1+sum(,2)");
  testexpr("1+sum(2,(,))");
  testexpr("1+sum(2,())");

  testexpr("-3^2",-9.0);
  testexpr("2.0^-2",0.25);
  testexpr("2^-2.0",0.25);
  testexpr("2.0**-2",0.25);
  testexpr("2**-2",0.25);
  testexpr("1+()");
  testexpr("2^3",8.);
  testexpr("3**2",9.);
  testexpr("2^3^2",512.);
  testexpr("4**2**3",65536.);
  testexpr("exp(log(20))",20.0);
  testexpr("log10(1000.0)",3.0);
  testexpr("log2(8.0)",3.0);
  testexpr("log1p(10.0)",2.3978952727983706694204);
  testexpr("pow(14.3,3.4)",8475.05504068721348);

  testexpr("float(0)",0.0);
  testexpr("double(14)",14.0);
  testexpr("float('hejsa14')");
  testexpr("float(\"hejsa14\")");
  testexpr("int('hejsa14')");
  testexpr("str('hejsa14')","hejsa14");
  testexpr("int(14.1)",14);
  testexpr("int(14.9)",14);
  testexpr("int(-14.1)",-14);
  testexpr("int(-14.9)",-14);
  testexpr("int('-14')",-14);
  testexpr("str(5)","5");
  testexpr("string(-5)","-5");
  testexpr("str(5.5)","5.500000");
  testexpr("float('14hejsa')");
  testexpr("float('1.4hejsa')");
  testexpr("int('14hejsa')");
  testexpr("float('')");
  testexpr("int('')");
  testexpr("float('1.2')",1.2);
  testexpr("int('-3')",-3);

  testexpr("5.0 % 2");
  testexpr("5 % 2.0");
  testexpr("'lala' % 2");
  testexpr("2 % 'lala'");
  testexpr("17 % 13",4);
  testexpr("-17 % 13",-4);
  testexpr("-17 % int(13.3)",-4);

  testexpr("12 && 2",1);
  testexpr("0 && 12",0);
  testexpr("0 && 0",0);
  testexpr("-1 && 0",0);
  testexpr("0 and 12",0);
  testexpr("0and 12",0);
  testexpr("12and 2",1);
  testexpr("211%2==0",0);
  testexpr("212%2==0",1);

  testexpr("0==0",1);
  testexpr("10==10",1);
  testexpr("-1==-1",1);
  testexpr("0.==0.",1);
  testexpr("10.==10.",1);
  testexpr("-1.==-1.",1);
  testexpr("0==0.",1);
  testexpr("10.==10",1);
  testexpr("-1==-1.",1);
  testexpr("'hello'=='hello'",1);
  testexpr("''==\"\"",1);
  testexpr("0==1",0);
  testexpr("2==1.0",0);
  testexpr("1.0==2",0);
  testexpr("1.0=='hello'");
  testexpr("1.0==''");
  testexpr("'hello'==0");

  testexpr("0!=0",0);
  testexpr("10!=10",0);
  testexpr("-1!=-1",0);
  testexpr("0.!=0.",0);
  testexpr("10.!=10.",0);
  testexpr("-1.!=-1.",0);
  testexpr("0!=0.",0);
  testexpr("10.!=10",0);
  testexpr("-1!=-1.",0);
  testexpr("'hello'!='hello'",0);
  testexpr("''!=\"\"",0);
  testexpr("0!=1",1);
  testexpr("2 != 1.0",1);
  testexpr("1.0!=2",1);
  testexpr("1.0!='hello'");
  testexpr("1.0!=''");
  testexpr("'hello' != 0");

  testexpr("0 < 0",0);
  testexpr("0 < 1",1);
  testexpr("1 < -1",0);
  testexpr("1 < -1.0",0);
  testexpr("1.0 < 2",1);
  testexpr("1 < 1.0",0);
  testexpr("'aaaa' < 'b'",1);
  testexpr("'aa' < 'aa'",0);
  testexpr("'' < 'a'",1);
  testexpr("'Aaaa' < 'aa'",1);
  testexpr("'Aaaa' < 2");

  testexpr("0 <= 0",1);
  testexpr("0 <= 1",1);
  testexpr("1 <= -1",0);
  testexpr("1 <= -1.0",0);
  testexpr("1.0 <= 2",1);
  testexpr("1 <= 1.0",1);
  testexpr("'aaaa' <= 'b'",1);
  testexpr("'aa' <= 'aa'",1);
  testexpr("'' <= 'a'",1);
  testexpr("'Aaaa' <= 'aa'",1);
  testexpr("'Aaaa' <= 2");

  testexpr("0 > 0",0);
  testexpr("0 > 1",0);
  testexpr("1 > -1",1);
  testexpr("1 > -1.0",1);
  testexpr("1.0 > 2",0);
  testexpr("1 > 1.0",0);
  testexpr("'aaaa' > 'b'",0);
  testexpr("'aa' > 'aa'",0);
  testexpr("'' > 'a'",0);
  testexpr("'Aaaa' > 'aa'",0);
  testexpr("'Aaaa' > 2");

  testexpr("0 >= 0",1);
  testexpr("0 >= 1",0);
  testexpr("1 >= -1",1);
  testexpr("1 >= -1.0",1);
  testexpr("1.0 >= 2",0);
  testexpr("1 >= 1.0",1);
  testexpr("'aaaa' >= 'b'",0);
  testexpr("'aa' >= 'aa'",1);
  testexpr("'' >= 'a'",0);
  testexpr("'Aaaa' >= 'aa'",0);
  testexpr("'Aaaa' >= 2");

  testexpr("10!=10",0);
  testexpr("-1!=-1",0);
  testexpr("0.!=0.",0);
  testexpr("10.!=10.",0);
  testexpr("-1.!=-1.",0);
  testexpr("0!=0.",0);
  testexpr("10.!=10",0);
  testexpr("-1!=-1.",0);
  testexpr("'hello'!='hello'",0);
  testexpr("''!=\"\"",0);
  testexpr("0!=1",1);
  testexpr("2 != 1.0",1);
  testexpr("1.0!=2",1);
  testexpr("1.0!='hello'");
  testexpr("1.0!=''");
  testexpr("'hello' != 0");

  testexpr("bool('hello')",1);
  testexpr("bool('')",0);
  testexpr("bool(0)",0);
  testexpr("bool(0.0)",0);
  testexpr("bool(10.0)",1);
  testexpr("bool(10)",1);

  testexpr("0xF0F0  |  0X00ff",0xf0ff);
  testexpr("0xF0F0  &  0X00ff",0x00f0);
  testexpr("0xF0F0 xor 0X00ff",0xf00f);
  testexpr("1 << 1",2);
  testexpr("2 << 1",4);
  testexpr("4 >> 1",2);
  testexpr("0xF0F << 4",0xf0F0);
  testexpr("0xF0F >> 4",0xf0);
  testexpr("0x5d06a << 7",0x2e83500);
  testexpr("~ 0xFFFFFFFFFFFFFFFF",0);
  testexpr("~ 0",(EP::int_type)0xFFFFFFFFFFFFFFFF);
  testexpr("~ 0xF0F0F0F0F0F0F0F0",(EP::int_type)0x0F0F0F0F0F0F0F0F);
  testexpr("isnan(17)",0);
  testexpr("isnan(17.0)",0);
  testexpr("isnan(nan)",1);
  testexpr("isnan(inf)",0);
  testexpr("isnan(-inf)",0);
  testexpr("isinf(17)",0);
  testexpr("isinf(17.0)",0);
  testexpr("isinf(nan)",0);
  testexpr("isinf(inf)",1);
  testexpr("isinf(-inf)",1);
  testexpr("isnormal(inf)",0);
  testexpr("isnormal(-inf)",0);
  testexpr("isnormal(nan)",0);
  testexpr("isnormal(17)",1);
  testexpr("isnormal(17.0)",1);
  testexpr("isfinite(inf)",0);
  testexpr("isfinite(-inf)",0);
  testexpr("isfinite(nan)",0);
  testexpr("isfinite(17)",1);
  testexpr("isfinite(17.0)",1);

  testexpr("ipow(3,3)",27);
  testexpr("ipow(0,1)",0);
  testexpr("ipow(1,0)",1);
  testexpr("ipow(10,0)",1);
  testexpr("ipow(-10,0)",1);
  testexpr("ipow(0,0)");
  testexpr("ipow(1,0)",1);
  testexpr("ipow(-1,0)",1);
  testexpr("ipow(1,-1)");

  testexpr("ipow(volatile(3),0)",1);
  testexpr("ipow(volatile(3),1)",3);
  testexpr("ipow(volatile(3),2)",9);
  testexpr("ipow(volatile(3),3)",27);
  testexpr("ipow(volatile(3),9)",19683);
  testexpr("ipow(volatile(3),10)",59049);

  testexpr("pow(volatile(3),0)",1.);
  testexpr("pow(volatile(3),1)",3.);
  testexpr("pow(volatile(3),2)",9.);
  testexpr("pow(volatile(3),3)",27.);
  testexpr("pow(volatile(3),9)",19683.);
  testexpr("pow(volatile(3),10)",59049.);

  testexpr("volatile(-3.0)^2",9.0);
  testexpr("volatile(3.0)^2",9.0);

  testexpr("ceil(-1.5)",-1.0);
  testexpr("ceil(-1.0)",-1.0);
  testexpr("ceil(-0.5)",0.);
  testexpr("ceil(0.0)",0.);
  testexpr("ceil(0.5)",1.0);
  testexpr("ceil(1.0)",1.0);
  testexpr("ceil(1.5)",2.0);
  testexpr("floor(-1.5)",-2.0);
  testexpr("floor(-1.0)",-1.0);
  testexpr("floor(-0.5)",-1.);
  testexpr("floor(0.0)",0.);
  testexpr("floor(0.5)",.0);
  testexpr("floor(1.0)",1.0);
  testexpr("floor(1.5)",1.0);
  testexpr("trunc(-1.5)",-1.0);
  testexpr("trunc(-1.0)",-1.0);
  testexpr("trunc(-0.5)",0.);
  testexpr("trunc(0.0)",0.);
  testexpr("trunc(0.5)",0.);
  testexpr("trunc(1.0)",1.0);
  testexpr("trunc(1.5)",1.0);
  testexpr("round(-1.5)",-2.0);
  testexpr("round(-1.0)",-1.0);
  testexpr("round(-0.5)",-1.0);
  testexpr("round(0.0)",0.);
  testexpr("round(0.5)",1.0);
  testexpr("round(1.0)",1.0);
  testexpr("round(1.5)",2.0);

  testexpr("min(5.5)",5.5);
  testexpr("min()");
  testexpr("min(1,2)",1);
  testexpr("min(1,2.0)",1.0);
  testexpr("min(1,2.0,3)",1.0);
  testexpr("min(2.2,1.0,-1.3)",-1.3);
  testexpr("min(2.2,1,-1.3)",-1.3);
  testexpr("min(2,1,-1,17,-18)",-18);
  testexpr("min(2,1,volatile(-1),17,volatile(-18))",-18);
  testexpr("min(volatile(2),volatile(1),volatile(-1),volatile(17),volatile(-18))",-18);

  testexpr("max(5.5)",5.5);
  testexpr("max()");
  testexpr("max(1,2)",2);
  testexpr("max(1.0,2)",2.0);
  testexpr("max(1.0,2,0)",2.0);
  testexpr("max(2.2,1.0,-1.3)",2.2);
  testexpr("max(2.2,1,-1.3)",2.2);
  testexpr("max(2,1,-1,17,-18)",17);

  testexpr("2*pi",2*M_PI);

  testexpr("lgamma(-1.5)",0.86004701537648087228405);
  testexpr("lgamma(4.5)",2.4537365708424423438316);
  testexpr("lgamma(1)",0.);
  testexpr("lgamma(2)",0.);
  testexpr("lgamma(3)",std::log(2.));
  testexpr("lgamma(4)",std::log(6.));
  testexpr("lgamma(5)",std::log(24.));
  testexpr("lgamma(1.)",0.);
  testexpr("lgamma(2.)",0.);
  testexpr("lgamma(3.)",std::log(2.));
  testexpr("lgamma(4.)",std::log(6.));
  testexpr("lgamma(5.)",std::log(24.));

  testexpr("tgamma(-1.5)",2.3632718012073548052854);
  testexpr("tgamma(4.5)",11.631728396567448058363);
  testexpr("tgamma(1)",1.);
  testexpr("tgamma(2)",1.);
  testexpr("tgamma(3)",2.);
  testexpr("tgamma(4)",6.);
  testexpr("tgamma(5)",24.);
  testexpr("tgamma(1.)",1.);
  testexpr("tgamma(2.)",1.);
  testexpr("tgamma(3.)",2.);
  testexpr("tgamma(4.)",6.);
  testexpr("tgamma(5.)",24.);

  testexpr("fabs(-3)",3.0);
  testexpr("fabs(0)",0.0);
  testexpr("fabs(3)",3.0);
  testexpr("abs(-3)",3);
  testexpr("abs(0)",0);
  testexpr("abs(3)",3);

  testexpr("fabs(-3.)",3.0);
  testexpr("fabs(-0.)",0.0);
  testexpr("fabs(0.)",0.0);
  testexpr("fabs(3.)",3.0);
  testexpr("abs(-3.)",3.);
  testexpr("abs(-0.)",0.);
  testexpr("abs(0.)",0.);
  testexpr("abs(3.)",3.);

  testexpr("5()");
  testexpr("5 2");
  testexpr(" ");
  testexpr(" ()");
  testexpr(" )");
  testexpr(" pi)");
  testexpr("blabla");
  testexpr("sin");
  testexpr("sin(2,3)");
  testexpr("sin()");
  testexpr("bla()");
  testexpr("sin(3,)");
  testexpr("sin(,3)");
  testexpr("2+sin(3,)");
  testexpr("sin(,)");
  testexpr("(,)");

  testexpr("5 +");
  testexpr("+ 5",5);
  testexpr("*");

  testexpr("volatile(4.0) > 5.0",0);
  testexpr("5.0 < volatile(4.0)",0);
  testexpr("volatile(6.0) > 5.0",1);
  testexpr("5.0 < volatile(6.0)",1);

  testexpr("volatile(4.0) >= 5.0",0);
  testexpr("5.0 <= volatile(4.0)",0);
  testexpr("volatile(6.0) >= 5.0",1);
  testexpr("5.0 <= volatile(6.0)",1);
  testexpr("volatile(5.0) >= 5.0",1);
  testexpr("5.0 <= volatile(5.0)",1);

  testexpr("volatile(4) > 5",0);
  testexpr("5 < volatile(4)",0);
  testexpr("volatile(6) > 5",1);
  testexpr("5 < volatile(6)",1);

  testexpr("volatile(4) >= 5",0);
  testexpr("5 <= volatile(4)",0);
  testexpr("volatile(6) >= 5",1);
  testexpr("5 <= volatile(6)",1);
  testexpr("volatile(5) >= 5",1);
  testexpr("5 <= volatile(5)",1);

  testexpr("5.01 != 5",1);
  testexpr("5.01 == 5",0);

  testexpr("volatile(5) == 6.1",0);
  testexpr("volatile(5) == 5.1",0);
  testexpr("volatile(5) != 6.1",1);
  testexpr("volatile(5) != 5.1",1);
  testexpr("volatile(5) == 6.0",0);
  testexpr("volatile(5) == 5.0",1);
  testexpr("volatile(5) != 6.0",1);
  testexpr("volatile(5) != 5.0",0);

  //64 bit ints can't reach +-1e19, so the following comparisons should evaluate
  //as constants:
  testexpr("volatile(5) <= -1.0e19",0);
  testexpr("volatile(5) < -1.0e19",0);
  testexpr("volatile(5) > -1.0e19",1);
  testexpr("volatile(5) >= -1.0e19",1);
  testexpr("volatile(5) == -1.0e19",0);
  testexpr("volatile(5) != -1.0e19",1);
  testexpr("volatile(5) <= 1.0e19",1);
  testexpr("volatile(5) < 1.0e19",1);
  testexpr("volatile(5) > 1.0e19",0);
  testexpr("volatile(5) >= 1.0e19",0);
  testexpr("volatile(5) == 1.0e19",0);
  testexpr("volatile(5) != 1.0e19",1);

  testexpr("pi",M_PI);
  testexpr("2 pi",2*M_PI);
  testexpr("2.0 pi",2*M_PI);
  testexpr("pi pi");
  testexpr("1/2pi",1/(2*M_PI));
  testexpr("2pi^2",2*M_PI*M_PI);

  testexpr("sin(1)",sin(1));
  testexpr("2sin(1)",2*sin(1));
  testexpr("2.0sin(1)",2.0*sin(1));
  testexpr("sin(1) sin(2)");
  testexpr("1/2sin(1)",1/(2*sin(1)));

  testexpr("0b9");
  testexpr("0xG");
  testexpr("09");
  testexpr("0b151");

  testexpr("1 and 0",0);
  testexpr("1 and 1",1);

  return 0;
}
