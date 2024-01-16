#include "ExprParser/ASTStdMath.hh"
#include <set>
#include <cmath>
#include <limits>
#include <cassert>
#include <sstream>

namespace ExprParser {

  // Particular operator implementations follows here - todo to separate file??

  template<class type_arg1, class type_arg2, class type_res=decltype(type_arg1()+type_arg2()) >
  class ExprEntity_BinaryAddition : public ExprEntity<type_res> {
  public:
    virtual str_type name() const {
      str_type s("BinAdd_");
      s+=exprTypeChar<type_arg1>();s+=exprTypeChar<type_arg2>();
      s+='2';s+=exprTypeChar<type_res>();
      return s;
    }

    ExprEntity_BinaryAddition(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<type_res>()
    {
      _ensure_type<type_arg1>(arg1);
      _ensure_type<type_arg2>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_BinaryAddition(){}
    virtual type_res evaluate() const {
      //todo: not very efficient implementation for strings...
      return _eval<type_arg1>(this->ExprEntityBase::child(0)) + _eval<type_arg2>(this->ExprEntityBase::child(1));
    }
  protected:

    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      //if one of the operands is a constant evalutation to 0 / 0.0 / "", the
      //result will basically equal the other operand. And additional
      //complication is that int-promotion to float still takes place (like in
      //5+0.0 -> 5.0), and since this method is called, the other argument must
      //be non-constant.
      //
      ExprEntityPtr c0(this->ExprEntityBase::child(0)), c1(this->ExprEntityBase::child(1));
      ExprEntityPtr res(0);
      if (c0->isConstant()&&_eval<type_arg1>(c0)==type_arg1())
        res = c1;//addition with c0 won't do anything.
      else if (c1->isConstant()&&_eval<type_arg2>(c1)==type_arg2())
        res = c0;//addition with c1 won't do anything.
      if (!res) {
        //TODO: If c1 is an unary minus, we can replace ourselves with a subtraction between c0 and c1->child(0).
        return res;
      }
      //ok, optimised version becomes res!
      if (res->returnType() != exprType<type_res>()) {
        //but res must be promoted (from int_type to float_type is in fact the
        //only scenario for addition, but we keep the code general)!
        res = create_typecast(res, exprType<type_res>());
      }
      return res;
    }
  };


  ExprEntityPtr create_binaryaddition(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();
    if (t1==ET_STRING||t2==ET_STRING) {
      if (t1!=ET_STRING||t2!=ET_STRING)
        EXPRPARSER_THROW(ParseError,"Incompatible types - attempt to add number to string");
      return makeobj<ExprEntity_BinaryAddition<str_type,str_type>>(arg1,arg2);
    } else {
      if (t1==ET_INT) {
        return t2==ET_INT ? makeobj<ExprEntity_BinaryAddition<int_type,int_type>>(arg1,arg2)
                          : makeobj<ExprEntity_BinaryAddition<int_type,float_type>>(arg1,arg2);
      } else {
        return t2==ET_INT ? makeobj<ExprEntity_BinaryAddition<float_type,int_type>>(arg1,arg2)
                          : makeobj<ExprEntity_BinaryAddition<float_type,float_type>>(arg1,arg2);
      }
    }
  }

  template<class TValue>
  class ExprEntity_UnaryMinus : public ExprEntity<TValue> {
  public:
    virtual str_type name() const {
      str_type s("UnaryMinus_");
      s += exprTypeChar<TValue>();
      return s;
    }
    ExprEntity_UnaryMinus(ExprEntityPtr arg)
      : ExprEntity<TValue>()
    {
      _ensure_type<TValue>(arg);
      ExprEntityBase::m_children.push_back(arg);
    }
    virtual ~ExprEntity_UnaryMinus(){}
    virtual TValue evaluate() const {
      //todo: not very efficient implementation for strings...
      return - _eval<TValue>(this->ExprEntityBase::child(0));
    }
  };

  ExprEntityPtr create_unaryminus(ExprEntityPtr arg)
  {
    auto t = arg->returnType();
    if (t==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - unary minus does not work on strings");
    if (t==ET_INT)
      return makeobj<ExprEntity_UnaryMinus<int_type>>(arg);
    _ensure_type<float_type>(arg);
    return makeobj<ExprEntity_UnaryMinus<float_type>>(arg);
  }

  ExprEntityPtr create_unaryplus(ExprEntityPtr arg)
  {
    if (arg->returnType()==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - unary plus does not work on strings");
    return arg;//doesn't do a thing on numbers obviously...
  }

  template<class type_arg1, class type_arg2, class type_res=decltype(type_arg1()-type_arg2()) >
  class ExprEntity_BinarySubtraction : public ExprEntity<type_res> {
  public:
    virtual str_type name() const {
      str_type s("BinSub_");
      s+=exprTypeChar<type_arg1>();s+=exprTypeChar<type_arg2>();
      s+='2';s+=exprTypeChar<type_res>();
      return s;
    }

    ExprEntity_BinarySubtraction(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<type_res>()
    {
      _ensure_type<type_arg1>(arg1);
      _ensure_type<type_arg2>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_BinarySubtraction(){}
    virtual type_res evaluate() const {
      return _eval<type_arg1>(this->ExprEntityBase::child(0)) - _eval<type_arg2>(this->ExprEntityBase::child(1));
    }
  protected:

    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      //if first operand is constant zero, we can replace the entity with unary
      //minus, and if the second operand is constant zero, we can replace with
      //the first entity. Like for addition, we also need to consider
      //preservation of ints to floats promotion.

      ExprEntityPtr c0(this->ExprEntityBase::child(0)), c1(this->ExprEntityBase::child(1));
      ExprEntityPtr res(0);
      bool need_unary_minus(false);
      if (c0->isConstant()&&_eval<type_arg1>(c0)==type_arg1()) {
        need_unary_minus = true;
        res = c1;
      } else if (c1->isConstant()&&_eval<type_arg2>(c1)==type_arg2()) {
        res = c0;
      }
      if (!res) {
        //TODO: If c1 is an unary minus, we can replace ourselves with an addition between c0 and c1->child(0).
        return res;
      }

      //ok, optimised version becomes res!
      if (res->returnType() != exprType<type_res>()) {
        //but res must be promoted (from int_type to float_type is in fact the
        //only scenario for subtraction, but we keep the code general)!
        res = create_typecast(res, exprType<type_res>());
      }
      if (need_unary_minus) {
        //and we need a lingering unary minus:
        res = makeobj<ExprEntity_UnaryMinus<type_res>>(res);
      }

      return res;
    }
  };

  ExprEntityPtr create_binarysubtraction(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();

    if (t1==ET_STRING||t2==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - subtraction does not work on strings");
    if (t1==ET_INT) {
      return t2==ET_INT ? makeobj<ExprEntity_BinarySubtraction<int_type,int_type>>(arg1,arg2)
        : makeobj<ExprEntity_BinarySubtraction<int_type,float_type>>(arg1,arg2);
    } else {
      return t2==ET_INT ? makeobj<ExprEntity_BinarySubtraction<float_type,int_type>>(arg1,arg2)
        : makeobj<ExprEntity_BinarySubtraction<float_type,float_type>>(arg1,arg2);
    }
  }

  template<class type_arg, unsigned power, class type_res=type_arg>
  class ExprEntity_FixedPower : public ExprEntity<type_res> {
  public:
    virtual str_type name() const {
      static_assert(power>=2&&power<=9,"invalid power");
      str_type s("FixedPow");
      s+=('0'+power); s+='_'; s+=exprTypeChar<type_arg>();
      s+='2';s+=exprTypeChar<type_res>();
      return s;
    }

    ExprEntity_FixedPower(ExprEntityPtr arg)
      : ExprEntity<type_res>()
    {
      _ensure_type<type_arg>(arg);
      ExprEntityBase::m_children.push_back(arg);
    }
    virtual ~ExprEntity_FixedPower(){}
    virtual type_res evaluate() const {
      type_res a = _eval<type_arg>(this->ExprEntityBase::child(0));
      if (power==2) { return a*a; }//1 mult
      else if (power==3) { return a*a*a; }//2 mult
      else if (power==4) { a *= a; return a*a; }//2 mult
      else if (power==5) { type_res b = a*a; return b*b*a; }//3 mult
      else if (power==6) { a *= a; return a*a*a; }//3 mult
      else if (power==7) { type_res b = a*a*a; return b*b*a; }//4 mult
      else if (power==8) { a *= a; a *= a; return a*a; }//3 mult
      else { assert(power==9); type_res b = a*a*a; return b*b*b; }
    }
  protected:
    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      //if power is even and child is UnaryMinus, we could bypass the UnaryMinus.
      return 0;
    }
  };

  template<class type_res>
  ExprEntityPtr create_fixedpower(ExprEntityPtr arg, unsigned power)
  {
    auto t = arg->returnType();
    if (t==ET_INT) {
      switch (power) {
      case 1: return create_typecast(arg,exprType<type_res>());
      case 2: return makeobj<ExprEntity_FixedPower<int_type,2,type_res>>(arg);
      case 3: return makeobj<ExprEntity_FixedPower<int_type,3,type_res>>(arg);
      case 4: return makeobj<ExprEntity_FixedPower<int_type,4,type_res>>(arg);
      case 5: return makeobj<ExprEntity_FixedPower<int_type,5,type_res>>(arg);
      case 6: return makeobj<ExprEntity_FixedPower<int_type,6,type_res>>(arg);
      case 7: return makeobj<ExprEntity_FixedPower<int_type,7,type_res>>(arg);
      case 8: return makeobj<ExprEntity_FixedPower<int_type,8,type_res>>(arg);
      case 9: return makeobj<ExprEntity_FixedPower<int_type,9,type_res>>(arg);
      }
    }
    if (t==ET_FLOAT&&exprType<type_res>()==exprType<float_type>()) {
      switch (power) {
      case 1: return create_typecast(arg,exprType<type_res>());
      case 2: return makeobj<ExprEntity_FixedPower<float_type,2,float_type>>(arg);
      case 3: return makeobj<ExprEntity_FixedPower<float_type,3,float_type>>(arg);
      case 4: return makeobj<ExprEntity_FixedPower<float_type,4,float_type>>(arg);
      case 5: return makeobj<ExprEntity_FixedPower<float_type,5,float_type>>(arg);
      case 6: return makeobj<ExprEntity_FixedPower<float_type,6,float_type>>(arg);
      case 7: return makeobj<ExprEntity_FixedPower<float_type,7,float_type>>(arg);
      case 8: return makeobj<ExprEntity_FixedPower<float_type,8,float_type>>(arg);
      case 9: return makeobj<ExprEntity_FixedPower<float_type,9,float_type>>(arg);
      }
    }
    EXPRPARSER_THROW(LogicError,"create_fixedpower called with invalid power or type");
  }

  template<class type_arg1, class type_arg2, class type_res=decltype(type_arg1()*type_arg2()) >
  class ExprEntity_BinaryMultiplication : public ExprEntity<type_res> {
  public:
    virtual str_type name() const {
      str_type s("BinMult_");
      s+=exprTypeChar<type_arg1>();s+=exprTypeChar<type_arg2>();
      s+='2';s+=exprTypeChar<type_res>();
      return s;
    }

    ExprEntity_BinaryMultiplication(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<type_res>()
    {
      _ensure_type<type_arg1>(arg1);
      _ensure_type<type_arg2>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_BinaryMultiplication(){}
    virtual type_res evaluate() const {
      return _eval<type_arg1>(this->ExprEntityBase::child(0)) * _eval<type_arg2>(this->ExprEntityBase::child(1));
    }
  protected:

    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      ExprEntityPtr c0(this->ExprEntityBase::child(0)), c1(this->ExprEntityBase::child(1));
      if (c0==c1)
        return create_fixedpower<type_res>(c0,2);//replace with c0^2

      //todo: if both children are UnaryMinus, we could bypass the UnaryMinuses
      //and go directly to the grandchildren (same for binarydivision!).

      bool const0(c0->isConstant());
      bool const1(c1->isConstant());
      //make sure to only evaluate constant children:
      type_arg1 v0 = const0 ? _eval<type_arg1>(c0) : type_arg1();
      type_arg2 v1 = const1 ? _eval<type_arg2>(c1) : type_arg2();

      //First check for pure integer multiplication with one of the arguments a
      //constant 0 (too bad FP math has signed zero and 0*infinity is
      //undefined - should we implement aggressive opt mode?):
      if ( exprType<type_arg1>() == exprType<int_type>() &&
           exprType<type_arg2>() == exprType<int_type>() ) {
        if (const0 && v0 == type_arg1(0))
          return c0;
        if (const1 && v1 == type_arg1(0))
          return c1;
      }

      //If any operand is constant 1, we can replace the entity with the other
      //operand. If any operand is constant -1, we can wrap the other operand in
      //an unary minus. Like for addition, we also need to consider preservation
      //of ints to floats promotion.

      ExprEntityPtr res(0);
      bool need_unary_minus(false);
      if (const0 && v0 == type_arg1(1)) {
        res = c1;
      } else if (const0 && v0 == type_arg1(-1)) {
        need_unary_minus = true;
        res = c1;
      } else if (const1 && v1 == type_arg2(1)) {
        res = c0;
      } else if (const1 && v1 == type_arg2(-1)) {
        need_unary_minus = true;
        res = c0;
      }

      if (!res)
        return res;

      //ok, optimised version becomes res!
      if (res->returnType() != exprType<type_res>()) {
        //but res must be promoted (from int_type to float_type is in fact the
        //only scenario for multiplication, but we keep the code general)!
        res = create_typecast(res, exprType<type_res>());
      }
      if (need_unary_minus) {
        //and we need a lingering unary minus:
        res = makeobj<ExprEntity_UnaryMinus<type_res>>(res);
      }

      return res;
    }
  };

  ExprEntityPtr create_binarymultiplication(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();
    if (t1==ET_STRING||t2==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - multiplication does not work on strings");
    if (t1==ET_INT) {
      return t2==ET_INT ? makeobj<ExprEntity_BinaryMultiplication<int_type,int_type>>(arg1,arg2)
        : makeobj<ExprEntity_BinaryMultiplication<int_type,float_type>>(arg1,arg2);
    } else {
      return t2==ET_INT ? makeobj<ExprEntity_BinaryMultiplication<float_type,int_type>>(arg1,arg2)
        : makeobj<ExprEntity_BinaryMultiplication<float_type,float_type>>(arg1,arg2);
    }
  }

  template<class type_arg1, class type_arg2, class type_res=decltype(type_arg1()/type_arg2(1)) >
  class ExprEntity_BinaryDivision : public ExprEntity<type_res> {
  public:
    virtual str_type name() const {
      str_type s("BinDiv_");
      s+=exprTypeChar<type_arg1>();s+=exprTypeChar<type_arg2>();
      s+='2';s+=exprTypeChar<type_res>();
      return s;
    }

    ExprEntity_BinaryDivision(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<type_res>()
    {
      _ensure_type<type_arg1>(arg1);
      _ensure_type<type_arg2>(arg2);
      if (arg2->isConstant() && _eval<type_arg2>(arg2)==type_arg2(0))
        EXPRPARSER_THROW(DomainError,"Division with value which is always zero");
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_BinaryDivision(){}
    virtual type_res evaluate() const {
      return _eval<type_arg1>(this->ExprEntityBase::child(0)) / _eval<type_arg2>(this->ExprEntityBase::child(1));
    }
  protected:

    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      ExprEntityPtr c0(this->ExprEntityBase::child(0)), c1(this->ExprEntityBase::child(1));
      bool const0(c0->isConstant());
      bool const1(c1->isConstant());
      type_arg1 v0 = const0 ? _eval<type_arg1>(c0) : type_arg1();
      type_arg2 v1 = const1 ? _eval<type_arg2>(c1) : type_arg2();

      const bool c0_int(exprType<type_arg1>() == exprType<int_type>());
      const bool c1_int(exprType<type_arg2>() == exprType<int_type>());
      assert_logic(c0_int||(exprType<type_arg1>() == exprType<float_type>()));
      assert_logic(c1_int||(exprType<type_arg2>() == exprType<float_type>()));

      //First check for pure integer division with a constant zero numerator:
      if ( c0_int && c1_int && const0 && v0 == type_arg1(0) )
        return c0;//(NB: in principle this can hide divide-by-zero errors in non-const c1 so perhaps only do it in aggresive opt mode?)

      //Division with constant 1:
      if (const1 && v1 == type_arg2(1)) {
        //division with 1 does nothing except the case where it is a float dividing an integer, thus promiting it:
        if (c0_int && !c1_int)
          return create_typecast(c0, exprType<type_res>());
        else
          return c0;
      }

      //Division with constant -1:
      if (const1 && v1 == type_arg2(-1)) {
        //division with -1 does nothing except changing the sign, and possibly promote the type:
        ExprEntityPtr res = makeobj<ExprEntity_UnaryMinus<type_arg1>>(c0);
        if (exprType<type_arg1>()!=exprType<type_res>())
          res = create_typecast(res, exprType<type_res>());
        return res;
      }

      //TODO: Turn division by float_type constant into multiplication with 1.0/constant. ? If aggressive opt...

      return ExprEntityPtr(0);
    }
  };

  ExprEntityPtr create_binarydivision(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();
    if (t1==ET_STRING||t2==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - division does not work on strings");
    if (t1==ET_INT) {
      return t2==ET_INT ? makeobj<ExprEntity_BinaryDivision<int_type,int_type>>(arg1,arg2)
        : makeobj<ExprEntity_BinaryDivision<int_type,float_type>>(arg1,arg2);
    } else {
      return t2==ET_INT ? makeobj<ExprEntity_BinaryDivision<float_type,int_type>>(arg1,arg2)
        : makeobj<ExprEntity_BinaryDivision<float_type,float_type>>(arg1,arg2);
    }
  }

  template<class type_arg1, class type_arg2>
  class ExprEntity_BinaryExponentiation : public ExprEntity<float_type> {
  public:
    virtual str_type name() const {
      str_type s("BinPow_");
      s+=exprTypeChar<type_arg1>();s+=exprTypeChar<type_arg2>();
      s+='2';s+=exprTypeChar<float_type>();
      return s;
    }

    ExprEntity_BinaryExponentiation(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<float_type>()
    {
      _ensure_type<type_arg1>(arg1);
      _ensure_type<type_arg2>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_BinaryExponentiation(){}
    virtual float_type evaluate() const {
      //Implementation (specialisation for int,int->int given below)
      return std::pow((float_type)_eval<type_arg1>(this->ExprEntityBase::child(0)),(float_type)_eval<type_arg2>(this->ExprEntityBase::child(1)));
    }
  private:
    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      //TODO:
      // pow(+1, exp) returns 1 for any exp, even when exp is NaN
      //                                  pow(base, ±0) returns 1 for any base, even when base is NaN
      //^2 / ^2.0 : special square impl? (perhaps special impl. for all integer n less than 10?
      //^0.5 / ^1/3 : special sqrt/cbrt?
      //
      //Also note std::cbrt which is like std::sqrt, but for the cubic root

      //TODO: return create_fixedpower<type_res>(ExprEntityPtr arg, unsigned power)

      if (child(1)->isConstant()) {
        float_type n = _eval<type_arg2>(this->ExprEntityBase::child(1));
        if (n>=1.0&&n<=9.0&&n==std::round(n))
          return create_fixedpower<float_type>(child(0), (int_type)std::round(n));
      }

      return 0;
    }
  };

  class ExprEntity_BinaryExponentiationIntegers : public ExprEntity<int_type> {
  public:
    virtual str_type name() const { return "BinPowInt_ii2i"; }

    ExprEntity_BinaryExponentiationIntegers(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<int_type>()
    {
      _ensure_type<int_type>(arg1);
      _ensure_type<int_type>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_BinaryExponentiationIntegers(){}
    virtual int_type evaluate() const
    {
      //a^n. 0^0 is undefined, as is negative n
      int_type a = _eval<int_type>(this->ExprEntityBase::child(0));
      int_type n = _eval<int_type>(this->ExprEntityBase::child(1));
      if (n<1) {
        if (n==0) {
          if (a==0)
            EXPRPARSER_THROW(DomainError,"0^0 is undefined");
          return 1;//a^0 for a!=0 is 1
        } else {
          EXPRPARSER_THROW(DomainError,"a^n is undefined for integers for negative n");
        }
      }
      int_type res = 1;
      while (n) {
        if (n & 1)
          res *= a;
        a *= a;
        n >>= 1;
      }
      return res;
    }
  private:
    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      if (child(1)->isConstant()) {
        int_type n = _eval<int_type>(this->ExprEntityBase::child(1));
        if (n>=1&&n<=9)
          return create_fixedpower<int_type>(child(0), n);
      }
      return 0;
    }
  };

  ExprEntityPtr create_binaryexponentiation(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();
    if (t1==ET_STRING||t2==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - exponentiation does not work on strings");
    if (t1==ET_INT) {
      return t2==ET_INT ? ExprEntityPtr( makeobj<ExprEntity_BinaryExponentiation<int_type,int_type>>(arg1,arg2) )
        : ExprEntityPtr( makeobj<ExprEntity_BinaryExponentiation<int_type,float_type>>(arg1,arg2) );
    } else {
      return t2==ET_INT ? ExprEntityPtr( makeobj<ExprEntity_BinaryExponentiation<float_type,int_type>>(arg1,arg2) )
        : ExprEntityPtr( makeobj<ExprEntity_BinaryExponentiation<float_type,float_type>>(arg1,arg2) );
    }
  }

  ExprEntityPtr create_binaryexponentiation_integers(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();
    if (t1!=ET_INT||t2!=ET_INT)
      EXPRPARSER_THROW(ParseError,"Incompatible types - integral exponentiation requires integral arguments");
    return makeobj<ExprEntity_BinaryExponentiationIntegers>(arg1,arg2);
  }

  class ExprEntity_Modulo : public ExprEntity<int_type> {
  public:
    virtual str_type name() const { return "BinModulo_ii2i"; }
    ExprEntity_Modulo(ExprEntityPtr arg1,ExprEntityPtr arg2)
      : ExprEntity<int_type>()
    {
      _ensure_type<int_type>(arg1);
      _ensure_type<int_type>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_Modulo(){}
    virtual int_type evaluate() const {
      return _eval<int_type>(this->ExprEntityBase::child(0)) % _eval<int_type>(this->ExprEntityBase::child(1));
    }
    //todo: 0 % anything is 0, anything % 1 is 0
  };

  ExprEntityPtr create_binarymodulo(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    if ( arg1->returnType()!=ET_INT || arg2->returnType()!=ET_INT )
      EXPRPARSER_THROW(ParseError,"Incompatible types - modulo operator only implemented for integers");
    return makeobj<ExprEntity_Modulo>(arg1,arg2);
  }

  template<class TValue>
  class ExprEntity_BooleanNot : public ExprEntity<int_type> {
  public:
    virtual str_type name() const {
      str_type s("BooleanNot_");
      s += exprTypeChar<TValue>();
      return s;
    }
    ExprEntity_BooleanNot(ExprEntityPtr arg)
      : ExprEntity<int_type>()
    {
      _ensure_type<TValue>(arg);
      ExprEntityBase::m_children.push_back(arg);
    }
    virtual ~ExprEntity_BooleanNot(){}
    virtual int_type evaluate() const {
      return _is_true(_eval<TValue>(this->ExprEntityBase::child(0))) ? 0l : 1l;
    }
  };

  ExprEntityPtr create_booleannot(ExprEntityPtr arg)
  {
    auto t = arg->returnType();
    if (t==ET_STRING)
      return makeobj<ExprEntity_BooleanNot<str_type>>(arg);
    if (t==ET_INT)
      return makeobj<ExprEntity_BooleanNot<int_type>>(arg);
    _ensure_type<float_type>(arg);
    return makeobj<ExprEntity_BooleanNot<float_type>>(arg);
  }

  template<class type_arg1, class type_arg2 >
  class ExprEntity_BooleanAnd : public ExprEntity<int_type> {
  public:
    virtual str_type name() const {
      str_type s("BooleanAnd_");
      s+=exprTypeChar<type_arg1>();
      s+=exprTypeChar<type_arg2>();
      s+="2i";
      return s;
    }

    ExprEntity_BooleanAnd(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<int_type>()
    {
      _ensure_type<type_arg1>(arg1);
      _ensure_type<type_arg2>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_BooleanAnd(){}
    virtual int_type evaluate() const {
      return _is_true(_eval<type_arg1>(this->ExprEntityBase::child(0))) ? _boolify(_eval<type_arg2>(this->ExprEntityBase::child(1))) : 0l;
    }
  protected:

    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      ExprEntityPtr c0(this->ExprEntityBase::child(0)), c1(this->ExprEntityBase::child(1));
      bool const0(c0->isConstant());
      bool const1(c1->isConstant());
      type_arg1 v0 = const0 ? _eval<type_arg1>(c0) : type_arg1();
      type_arg2 v1 = const1 ? _eval<type_arg2>(c1) : type_arg2();

      if (!const0 && !const1) {
        //TODO: No obvious optimisations here, but for any (grand)daughter
        //wrapped in ExprEntity_UnaryBool, we could get rid of the wrapper.
      }

      if (const0) {
        if (_is_true<type_arg1>(v0)) {
          return create_unarybool(c1);
        } else {
          //always false, even if arg2 is volatile!
          return create_constant((int_type)0l);
        }
      }
      if (const1) {
        if (_is_true<type_arg2>(v1)) {
          return create_unarybool(c0);
        } else {
          //always false, even if arg1 is volatile!
          //Note that optimising like this means that we won't evaluate arg1 at
          //all - which is ok as long as we don't allow side-effects (we could
          //add hasSideEffects() as property like isConstant()).
          return create_constant((int_type)0l);
        }
      }
      return ExprEntityPtr(0);
    }
  };

  ExprEntityPtr create_booleanand(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    //a bit tedious dispatching, could create helper template
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();
    if (t1==ET_STRING) {
      if (t2==ET_STRING)
        return makeobj<ExprEntity_BooleanAnd<str_type,str_type>>(arg1,arg2);
      return t2==ET_INT ? makeobj<ExprEntity_BooleanAnd<str_type,int_type>>(arg1,arg2)
        : makeobj<ExprEntity_BooleanAnd<str_type,float_type>>(arg1,arg2);
    }
    if (t1==ET_INT) {
      if (t2==ET_INT)
        return makeobj<ExprEntity_BooleanAnd<int_type,int_type>>(arg1,arg2);
      return t2==ET_STRING ? makeobj<ExprEntity_BooleanAnd<int_type,str_type>>(arg1,arg2)
        : makeobj<ExprEntity_BooleanAnd<int_type,float_type>>(arg1,arg2);
    }
    assert_logic(t1==ET_FLOAT);
    if (t2==ET_FLOAT)
      return makeobj<ExprEntity_BooleanAnd<float_type,float_type>>(arg1,arg2);
    return t2==ET_STRING ? makeobj<ExprEntity_BooleanAnd<float_type,str_type>>(arg1,arg2)
      : makeobj<ExprEntity_BooleanAnd<float_type,int_type>>(arg1,arg2);
  }

  template<class type_arg1, class type_arg2 >
  class ExprEntity_BooleanOr : public ExprEntity<int_type> {
  public:
    virtual str_type name() const {
      str_type s("BooleanOr_");
      s+=exprTypeChar<type_arg1>();
      s+=exprTypeChar<type_arg2>();
      s+="2i";
      return s;
    }

    ExprEntity_BooleanOr(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<int_type>()
    {
      _ensure_type<type_arg1>(arg1);
      _ensure_type<type_arg2>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_BooleanOr(){}
    virtual int_type evaluate() const {
      return _is_true(_eval<type_arg1>(this->ExprEntityBase::child(0))) ? 1l : _boolify(_eval<type_arg2>(this->ExprEntityBase::child(1)));
    }
  protected:

    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      ExprEntityPtr c0(this->ExprEntityBase::child(0)), c1(this->ExprEntityBase::child(1));
      bool const0(c0->isConstant());
      bool const1(c1->isConstant());
      type_arg1 v0 = const0 ? _eval<type_arg1>(c0) : type_arg1();
      type_arg2 v1 = const1 ? _eval<type_arg2>(c1) : type_arg2();

      if (!const0 && !const1) {
        //TODO: No obvious optimisations here, but for any (grand)daughter
        //wrapped in ExprEntity_UnaryBool, we could get rid of the wrapper.
      }

      if (const0) {
        if (_is_true<type_arg1>(v0)) {
          //always true, even if arg2 is volatile!
          return create_constant((int_type)1l);
        } else {
          return create_unarybool(c1);
        }
      }
      if (const1) {
        if (_is_true<type_arg2>(v1)) {
          //always true, even if arg1 is volatile!
          //Note that optimising like this means that we won't evaluate arg1 at
          //all - which is ok as long as we don't allow side-effects (we could
          //add hasSideEffects() as property like isConstant()).
          return create_constant((int_type)1l);
        } else {
          return create_unarybool(c0);
        }
      }
      return ExprEntityPtr(0);
    }
  };

  ExprEntityPtr create_booleanor(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    //a bit tedious dispatching, could create helper template
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();
    if (t1==ET_STRING) {
      if (t2==ET_STRING)
        return makeobj<ExprEntity_BooleanOr<str_type,str_type>>(arg1,arg2);
      return t2==ET_INT ? makeobj<ExprEntity_BooleanOr<str_type,int_type>>(arg1,arg2)
        : makeobj<ExprEntity_BooleanOr<str_type,float_type>>(arg1,arg2);
    }
    if (t1==ET_INT) {
      if (t2==ET_INT)
        return makeobj<ExprEntity_BooleanOr<int_type,int_type>>(arg1,arg2);
      return t2==ET_STRING ? makeobj<ExprEntity_BooleanOr<int_type,str_type>>(arg1,arg2)
        : makeobj<ExprEntity_BooleanOr<int_type,float_type>>(arg1,arg2);
    }
    assert_logic(t1==ET_FLOAT);
    if (t2==ET_FLOAT)
      return makeobj<ExprEntity_BooleanOr<float_type,float_type>>(arg1,arg2);
    return t2==ET_STRING ? makeobj<ExprEntity_BooleanOr<float_type,str_type>>(arg1,arg2)
      : makeobj<ExprEntity_BooleanOr<float_type,int_type>>(arg1,arg2);
  }

  template<class type_arg> bool _valIsFloatAndNaN(const type_arg&) { return false; }
  template<> bool _valIsFloatAndNaN<float_type>(const float_type& val) { return std::isnan(val); }

  template<class type_arg> bool _valIsFloatAndNotWholeNumber(const type_arg&) { return false; }
  template<> bool _valIsFloatAndNotWholeNumber<float_type>(const float_type& val) { return std::floor(val)!=val; }

  template<class type_arg, class type_cmpval, int ICMP>
  class ExprEntity_CmpVersusConst : public ExprEntity<int_type> {
    //Comparing with a constant value on the right hand side.
  public:

    explicit ExprEntity_CmpVersusConst(ExprEntityPtr arg, type_cmpval cmpval)
      : ExprEntity<int_type>(), m_cmpval(cmpval)
    {
      _ensure_type<type_arg>(arg);
      ExprEntityBase::m_children.push_back(arg);
    }
    virtual ~ExprEntity_CmpVersusConst(){}

    virtual str_type name() const {
      static_assert(ICMP>=0&&ICMP<=5,"implementation error");
      std::ostringstream ss;
      ss << "ConstCmp";
      if (ICMP==0) ss << "Equal";
      if (ICMP==1) ss << "NotEqual";
      if (ICMP==2) ss << "LT";
      if (ICMP==3) ss << "GT";
      if (ICMP==4) ss << "LE";
      if (ICMP==5) ss << "GE";
      ss << '_' << exprTypeChar<type_arg>() << 'v' << m_cmpval << exprTypeChar<type_cmpval>();
      return ss.str();
    }

    virtual int_type evaluate() const {
      static_assert(ICMP>=0&&ICMP<=5,"implementation error");
      const type_arg a = _eval<type_arg>(this->ExprEntityBase::child(0));
      if (ICMP==0) return a == m_cmpval ? 1l : 0l;
      if (ICMP==1) return a != m_cmpval ? 1l : 0l;
      if (ICMP==2) return a  < m_cmpval ? 1l : 0l;
      if (ICMP==3) return a  > m_cmpval ? 1l : 0l;
      if (ICMP==4) return a <= m_cmpval ? 1l : 0l;
      if (ICMP==5) return a >= m_cmpval ? 1l : 0l;
      return 0l;
    }
  protected:
    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      if ( _valIsFloatAndNaN(m_cmpval) )
        return create_constant((int_type)0l);//NaN's always compare false

      if (exprType<type_arg>()==ET_FLOAT&&exprType<type_cmpval>()==ET_INT) {
        //avoid having to promote cmpval from int to float on each evaluation:
        return makeobj<ExprEntity_CmpVersusConst<type_arg,type_arg,ICMP>>(child(0),(type_arg)m_cmpval);
      }

      if (exprType<type_arg>()==ET_INT&&exprType<type_cmpval>()==ET_FLOAT) {
        //We are comparing a volatile integer result with a float.
        //
        //Sometimes the float will be outside the possible values represented by integers:
        bool cv_toohigh(m_cmpval>static_cast<type_cmpval>(std::numeric_limits<type_arg>::max()));
        bool cv_toolow(m_cmpval<static_cast<type_cmpval>(std::numeric_limits<type_arg>::min()));
        bool cv_nonint(cv_toohigh||cv_toolow||_valIsFloatAndNotWholeNumber(m_cmpval));

        //If comparing == or != with a non-integral or out-of-range float value,
        //the result will always be the same:
        if ( (ICMP==0||ICMP==1) && cv_nonint )
          return create_constant( (int_type)(ICMP==0 ? 0l : 1l) );
        if (ICMP==2||ICMP==4) {
          //int <[=] cmpval
          if (cv_toolow) return create_constant( (int_type)0l );
          if (cv_toohigh) return create_constant( (int_type)1l );
        }
        if (ICMP==3||ICMP==5) {
          //int >[=] cmpval
          if (cv_toolow) return create_constant( (int_type)1l );
          if (cv_toohigh) return create_constant( (int_type)0l );
        }
      }

      //TODO: If type_arg = int, type_cmpval = float: We can transform into a
      //pure integer comparison, e.g. int > 5.5 or int >= 5.5 are both the same
      //as int > 5. Of course, things could get tricky near INT_MAX, INT_MIN.

      return 0;
    }

  private:
    type_cmpval m_cmpval;
  };

  template<class type_arg1, class type_arg2, int ICMP>
  class ExprEntity_Cmp : public ExprEntity<int_type> {
  public:
    ExprEntity_Cmp(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<int_type>()
    {
      _ensure_type<type_arg1>(arg1);
      _ensure_type<type_arg2>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_Cmp(){}

    virtual str_type name() const {
      static_assert(ICMP>=0&&ICMP<=5,"implementation error");
      str_type s("Cmp");
      if (ICMP==0) s += "Equal";
      if (ICMP==1) s += "NotEqual";
      if (ICMP==2) s += "LT";
      if (ICMP==3) s += "GT";
      if (ICMP==4) s += "LE";
      if (ICMP==5) s += "GE";
      s+="_"; s+=exprTypeChar<type_arg1>();
      s+="v"; s+=exprTypeChar<type_arg2>();
      return s;
    }

    virtual int_type evaluate() const {
      static_assert(ICMP>=0&&ICMP<=5,"implementation error");
      const type_arg1 a1 = _eval<type_arg1>(this->ExprEntityBase::child(0));
      const type_arg2 a2 = _eval<type_arg2>(this->ExprEntityBase::child(1));
      if (ICMP==0) return a1 == a2 ? 1l : 0l;
      if (ICMP==1) return a1 != a2 ? 1l : 0l;
      if (ICMP==2) return a1  < a2 ? 1l : 0l;
      if (ICMP==3) return a1  > a2 ? 1l : 0l;
      if (ICMP==4) return a1 <= a2 ? 1l : 0l;
      if (ICMP==5) return a1 >= a2 ? 1l : 0l;
      return 0l;
    }

  protected:

    // SLC6 misisng constexpr, hardcoding swapdir_icmp below in code (todo, change back!)
    // constexpr static int swapdir_icmp()
    // {
    //   static_assert(ICMP>=0&&ICMP<=5,"implementation error");
    //   return (ICMP<=1?ICMP:(ICMP==2?3:(ICMP==3?2:(ICMP==4?5:4))));
    // }

    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      ExprEntityPtr c0(this->ExprEntityBase::child(0)), c1(this->ExprEntityBase::child(1));
      bool const0(c0->isConstant());
      bool const1(c1->isConstant());
      assert_logic(! (const0 && const1) );//no need to call this method then...
      if (!const0 && !const1)
        return 0;//boths sides are volatile, no obvious optimisations.

      //Ok, exactly one side of our comparison is volatile and one is a constant!

      if (exprType<type_arg1>()!=exprType<type_arg2>()&&(exprType<type_arg1>()==ET_STRING||exprType<type_arg2>()==ET_STRING))
        EXPRPARSER_THROW(LogicError,"comparing strings to non-strings");//should not happen in principle...

      if (const1) {
        //comparison with a fixed value on the right hand side:
        return makeobj<ExprEntity_CmpVersusConst<type_arg1,type_arg2,ICMP>>(c0,_eval<type_arg2>(c1));
      } else {
        assert_logic(const0);
        //same, but swap operator so the fixed value ends up on the right hand side:
        const int swapdir_icmp = (ICMP<=1?ICMP:(ICMP==2?3:(ICMP==3?2:(ICMP==4?5:4))));
        return makeobj<ExprEntity_CmpVersusConst<type_arg2,type_arg1,swapdir_icmp>>(c1,_eval<type_arg1>(c0));
      }

    }

  };

  template <int icmp>
  ExprEntityPtr create_cmp(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();
    if (t1==ET_STRING||t2==ET_STRING) {
      if (t1==ET_STRING&&t2==ET_STRING)
        return makeobj<ExprEntity_Cmp<str_type,str_type,icmp>>(arg1,arg2);
      else
        EXPRPARSER_THROW(ParseError,"Incompatible types - can not compare strings with numbers");
    }
    if (t1==ET_INT) {
      if (t2==ET_INT) return makeobj<ExprEntity_Cmp<int_type,int_type,icmp>>(arg1,arg2);
      else return makeobj<ExprEntity_Cmp<int_type,float_type,icmp>>(arg1,arg2);
    } else {
      if (t2==ET_INT) return makeobj<ExprEntity_Cmp<float_type,int_type,icmp>>(arg1,arg2);
      else return makeobj<ExprEntity_Cmp<float_type,float_type,icmp>>(arg1,arg2);
    }
  }

  ExprEntityPtr create_cmp_equal(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_cmp<0>(arg1,arg2); }
  ExprEntityPtr create_cmp_notequal(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_cmp<1>(arg1,arg2); }
  ExprEntityPtr create_cmp_lt(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_cmp<2>(arg1,arg2); }
  ExprEntityPtr create_cmp_gt(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_cmp<3>(arg1,arg2); }
  ExprEntityPtr create_cmp_le(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_cmp<4>(arg1,arg2); }
  ExprEntityPtr create_cmp_ge(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_cmp<5>(arg1,arg2); }

  template<int IBITWISE>
  class ExprEntity_Bitwise : public ExprEntity<int_type> {
  public:
    ExprEntity_Bitwise(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<int_type>()
    {
      _ensure_type<int_type>(arg1);
      _ensure_type<int_type>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_Bitwise(){}

    virtual str_type name() const {
      static_assert(IBITWISE>=0&&IBITWISE<=4,"implementation error");
      str_type s("Bitwise");
      if (IBITWISE==0) s += "And";
      if (IBITWISE==1) s += "Or";
      if (IBITWISE==2) s += "Xor";
      if (IBITWISE==3) s += "LShift";
      if (IBITWISE==4) s += "RShift";
      return s;
    }

    virtual int_type evaluate() const {
      static_assert(IBITWISE>=0&&IBITWISE<=4,"implementation error");
      const std::uint64_t a1 = (std::uint64_t)_eval<int_type>(this->ExprEntityBase::child(0));
      const std::uint64_t a2 = (std::uint64_t)_eval<int_type>(this->ExprEntityBase::child(1));
      if (IBITWISE==0) return (int_type) ( a1 & a2 );
      if (IBITWISE==1) return (int_type) ( a1 | a2 );
      if (IBITWISE==2) return (int_type) ( a1 ^ a2 );
      if (IBITWISE==3) return (int_type) ( a1 << a2 );
      if (IBITWISE==4) return (int_type) ( a1 >> a2 );
      assert(false);
      return 0l;
    }
  };

  template <int ibitwise>
  ExprEntityPtr create_bitwise(ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();
    if (t1!=ET_INT||t2!=ET_INT)
      EXPRPARSER_THROW(ParseError,"Incompatible types - bitwise operators only work on integers");
    return makeobj<ExprEntity_Bitwise<ibitwise>>(arg1,arg2);
  }

  ExprEntityPtr create_bitwise_and(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_bitwise<0>(arg1,arg2); }
  ExprEntityPtr create_bitwise_or(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_bitwise<1>(arg1,arg2); }
  ExprEntityPtr create_bitwise_xor(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_bitwise<2>(arg1,arg2); }
  ExprEntityPtr create_bitwise_lshift(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_bitwise<3>(arg1,arg2); }
  ExprEntityPtr create_bitwise_rshift(ExprEntityPtr arg1, ExprEntityPtr arg2) { return create_bitwise<4>(arg1,arg2); }


  class ExprEntity_BitwiseNot : public ExprEntity<int_type> {
  public:
    ExprEntity_BitwiseNot(ExprEntityPtr arg)
      : ExprEntity<int_type>()
    {
      _ensure_type<int_type>(arg);
      ExprEntityBase::m_children.push_back(arg);
    }
    virtual ~ExprEntity_BitwiseNot(){}

    virtual str_type name() const {
      str_type s("BitwiseNot");
      return s;
    }

    virtual int_type evaluate() const {
      const std::uint64_t val = (std::uint64_t)_eval<int_type>(this->ExprEntityBase::child(0));
      return (int_type) ( ~val );
    }
  };

  ExprEntityPtr create_bitwise_not(ExprEntityPtr arg)
  {
    return makeobj<ExprEntity_BitwiseNot>(arg);
  }

  ExprEntityPtr create_function_sum(ExprEntityList& args)
  {
    for (auto& a : args) {
      if (a->returnType()==ET_STRING)
        EXPRPARSER_THROW(ParseError,"Incompatible types - function sum not implemented for strings");
    }
    if (args.size()==0)
      return makeobj<ExprEntityConstantValue<int_type>>(0l);
    else if (args.size()==1)
      return args[0];
    else if (args.size()==2)
      return create_binaryaddition(args[0],args[1]);
    //Handle >2 args via recursion (easy, but could be implemented more
    //efficiently!).  We take care respect operator + left-associativity,
    //i.e.: sum(a,b,c,...) = a + b + c + ... = ((a + b) + c) + ...
    auto arg_last = args.back();
    args.pop_back();
    return create_binaryaddition(create_function_sum(args),arg_last);
  }

  template<class type_arg1, class type_arg2, bool ismin, class type_res=decltype(type_arg1()+type_arg2())>
  class ExprEntity_MinMax : public ExprEntity<type_res> {
  public:
    virtual str_type name() const {
      str_type s(ismin?"BinMin_":"BinMax_");
      s+=exprTypeChar<type_arg1>();s+=exprTypeChar<type_arg2>();
      s+='2';s+=exprTypeChar<type_res>();
      return s;
    }
    ExprEntity_MinMax(ExprEntityPtr arg1, ExprEntityPtr arg2)
      : ExprEntity<type_res>()
    {
      _ensure_type<type_arg1>(arg1);
      _ensure_type<type_arg2>(arg2);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }
    virtual ~ExprEntity_MinMax(){}
    virtual type_res evaluate() const {
      type_arg1 a1 = _eval<type_arg1>(this->ExprEntityBase::child(0));
      type_arg2 a2 = _eval<type_arg2>(this->ExprEntityBase::child(1));
      if (ismin)
        return a1 < a2 ? a1 : a2;
      else
        return a1 < a2 ? a2 : a1;
    }
  };

  template<class type_res, bool ismin>
  void _prune_args_for_minmax(ExprEntityList& args)
  {
    //Removes any duplicate args and all but 1 of the constant args (the one min
    //or max would pick anyway).
    std::set<ExprEntityPtr> accepted;
    bool first_const(true);
    type_res v_minmax(0);
    ExprEntityPtr best_const(0);
    for( size_t i = 0; i < args.size(); ++i ) {
      auto& a = args[i];
      if (!a->isConstant()) {
        accepted.insert(a);
        continue;
      }
      type_res va = a->returnType()==ET_INT ? _eval<int_type>(a) : _eval<float_type>(a);
      if (first_const) {
        first_const = false;
      } else {
        if (ismin) { if (va >= v_minmax) continue; }
        if (!ismin) { if (va <= v_minmax) continue; }
      }
      best_const = a;
      v_minmax = va;
    }
    if (best_const)
      accepted.insert(best_const);
    if (accepted.size()!=args.size()) {
      //Remove elements from args, taking care to preserve ordering from args -
      //ordering in the accepted set depends on (randomized) pointer values.
      assert_logic(accepted.size()>0);
      ExprEntityList out;
      out.reserve(accepted.size());
      for (auto& a : args) {
        if (accepted.count(a)) {
          accepted.erase(a);
          out.push_back(a);
        }
      }
      std::swap(args,out);
    }
  }


  template<int ismin>
  ExprEntityPtr create_function_minmax(ExprEntityList& args)
  {
    //Run through arguments to figure out return type (float if any arg is
    //float), and error on string input. Also, if at least 2 args are constant
    //we can ignore all except the largest (if max) or smallest (if min) of
    //them (but remember that max(2,1.0) is 2.0, not 2).

    bool pure_int(true);
    for (auto& a : args) {
      if (a->returnType()==ET_STRING)
        EXPRPARSER_THROW(ParseError,"Incompatible types - functions min and max are not implemented for strings");
      if (a->returnType()==ET_FLOAT)
        pure_int = false;
    }

    if (args.empty())
      EXPRPARSER_THROW(ParseError,"Functions min and max requires at least one argument");

    //Prune duplicate args and all except the largest (if max) or smallest (if
    //min) of any constant args:
    size_t test_bef = args.size();
    if (pure_int)
      _prune_args_for_minmax<int_type,ismin>(args);
    else
      _prune_args_for_minmax<float_type,ismin>(args);
    if (test_bef != args.size())

    assert_logic(!args.empty());

    if (args.size()==1) {
      if (!pure_int && args[0]->returnType()==ET_INT)
        return create_typecast(args[0],ET_FLOAT);
      else
        return args[0];
    } else if (args.size()==2) {
      auto t1 = args[0]->returnType();
      auto t2 = args[1]->returnType();
      if (t1==ET_INT) {
        if (t2==ET_INT) return makeobj<ExprEntity_MinMax<int_type,int_type,ismin>>(args[0],args[1]);
        else return makeobj<ExprEntity_MinMax<int_type,float_type,ismin>>(args[0],args[1]);
      } else {
        if (t2==ET_INT) return makeobj<ExprEntity_MinMax<float_type,int_type,ismin>>(args[0],args[1]);
        else return makeobj<ExprEntity_MinMax<float_type,float_type,ismin>>(args[0],args[1]);
      }
    }
    assert_logic(args.size()>2);
    //Handle >2 args via recursion (easy, but could be implemented more
    //efficiently!).
    ExprEntityPtr arg_last = args.back();
    args.pop_back();
    ExprEntityList args2;
    args2.push_back(create_function_minmax<ismin>(args));
    args2.push_back(arg_last);
    return create_function_minmax<ismin>(args2);
  }

  ExprEntityPtr create_function_min(ExprEntityList& args) { return create_function_minmax<1>(args); }
  ExprEntityPtr create_function_max(ExprEntityList& args) { return create_function_minmax<0>(args); }



  template<class type_arg, class type_res = type_arg>
  class ExprEntity_Abs : public ExprEntity<type_res> {
  public:
    virtual str_type name() const {
      str_type s("Abs_");
      s+=exprTypeChar<type_arg>();
      s+='2';
      s+=exprTypeChar<type_res>();
      return s;
    }
    ExprEntity_Abs(ExprEntityPtr arg)
      : ExprEntity<type_res>()
    {
      _ensure_type<type_arg>(arg);
      ExprEntityBase::m_children.push_back(arg);
    }
    virtual ~ExprEntity_Abs(){}
    virtual type_res evaluate() const {
      //need f2f, i2f, i2i not f2i
      type_arg a = _eval<type_arg>(this->ExprEntityBase::child(0));
      if (exprType<type_arg>()==ET_FLOAT && exprType<type_res>()==ET_FLOAT)
        return fabs((float_type)a);//f2f
      if (exprType<type_arg>()==ET_INT && exprType<type_res>()==ET_FLOAT)
        return fabs((float_type)a);//i2f
      if (exprType<type_arg>()==ET_INT && exprType<type_res>()==ET_INT)
        return a < 0 ? -a : a;//i2i, overflows on INT_MIN
      EXPRPARSER_THROW(LogicError,"abs(..) not available for given types");
    }
  };

  ExprEntityPtr create_function_abs(ExprEntityPtr arg) {
    if (arg->returnType()==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - functions abs not implemented for strings");
    if (arg->returnType()==ET_INT)
      return makeobj<ExprEntity_Abs<int_type,int_type>>(arg);
    return makeobj<ExprEntity_Abs<float_type,float_type>>(arg);
  }

  ExprEntityPtr create_function_fabs(ExprEntityPtr arg) {
    if (arg->returnType()==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - functions abs not implemented for strings");
    if (arg->returnType()==ET_INT)
      return makeobj<ExprEntity_Abs<int_type,float_type>>(arg);
    return makeobj<ExprEntity_Abs<float_type,float_type>>(arg);
  }

  ExprEntityPtr create_std_math_constant(const str_type& name)
  {
    if (name=="pi")
      return makeobj<ExprEntityConstantValue<float_type>>(M_PI);
    else if (name=="e")
      return makeobj<ExprEntityConstantValue<float_type>>(M_E);
    else if (name=="inf")
      return makeobj<ExprEntityConstantValue<float_type>>(std::numeric_limits<float_type>::infinity());
    else if (name=="nan")
      return makeobj<ExprEntityConstantValue<float_type>>(std::numeric_limits<float_type>::quiet_NaN());
    else if (name=="true"||name=="True")
      return makeobj<ExprEntityConstantValue<int_type>>((int_type)1);
    else if (name=="false"||name=="False")
      return makeobj<ExprEntityConstantValue<int_type>>((int_type)0);
    return 0;

  }

  template<class type_arg0, class type_arg1, class type_arg2>
  class ExprEntity_InConstRangeFunc : public ExprEntity<int_type> {
  public:
    ExprEntity_InConstRangeFunc(ExprEntityPtr arg0,type_arg1 v1, type_arg2 v2)
      : ExprEntity<int_type>(), m_v1(v1), m_v2(v2)
    {
      assert_logic(m_v1<m_v2);//if not, we should have been replaced with constant "0" instead.
      _ensure_type<type_arg0>(arg0);
      ExprEntityBase::m_children.push_back(arg0);
    }

    virtual str_type name() const
    {
      str_type s("inconstrange_");
      s += exprTypeChar<type_arg0>();
      s += exprTypeChar<type_arg1>();
      s += exprTypeChar<type_arg2>();
      s += "2i";
      return s;
    }
    virtual ~ExprEntity_InConstRangeFunc(){}
    virtual int_type evaluate() const {
      type_arg0 v = _eval<type_arg0>(this->ExprEntityBase::child(0));
      return (v >= m_v1 && v < m_v2) ? 1 : 0;
    }
  private:
    type_arg1 m_v1;
    type_arg2 m_v2;
  };

  template<class type_arg0, class type_arg1, class type_arg2>
  class ExprEntity_InRangeFunc : public ExprEntity<int_type> {
  public:
    ExprEntity_InRangeFunc(ExprEntityPtr arg0,ExprEntityPtr arg1,ExprEntityPtr arg2)
      : ExprEntity<int_type>()
    {
      // m_name += '_';
      // m_name += exprTypeChar<type_arg>();
      // m_name += '2';
      // m_name += exprTypeChar<type_res>();
      _ensure_type<type_arg0>(arg0);
      _ensure_type<type_arg1>(arg1);
      _ensure_type<type_arg2>(arg2);
      ExprEntityBase::m_children.push_back(arg0);
      ExprEntityBase::m_children.push_back(arg1);
      ExprEntityBase::m_children.push_back(arg2);
    }

    virtual str_type name() const
    {
      str_type s("inrange_");
      s += exprTypeChar<type_arg0>();
      s += exprTypeChar<type_arg1>();
      s += exprTypeChar<type_arg2>();
      s += "2i";
      return s;
    }
    virtual ~ExprEntity_InRangeFunc(){}
    virtual int_type evaluate() const {
      type_arg0 v = _eval<type_arg0>(this->ExprEntityBase::child(0));
      return v >= _eval<type_arg1>(this->ExprEntityBase::child(1))
        ? (v <= _eval<type_arg2>(this->ExprEntityBase::child(2))?1:0)
        : 0;
    }

    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      ExprEntityPtr c0(this->ExprEntityBase::child(0)), c1(this->ExprEntityBase::child(1)), c2(this->ExprEntityBase::child(2));

      if (!c1->isConstant() || !c2->isConstant())
        return 0;//we dont implement any optimisations in this case

      type_arg1 v1 = _eval<type_arg1>(c1);
      type_arg2 v2 = _eval<type_arg2>(c2);

      if (v1 > v2)
        return create_constant((int_type)0l);//can never >=v1 and <= v2

      auto t0 = exprType<type_arg0>();
      auto t1 = exprType<type_arg1>();
      auto t2 = exprType<type_arg2>();

      assert_logic(t0!=ET_STRING&&t1!=ET_STRING&&t2!=ET_STRING);

      if (t0==ET_INT) {
        if (t1==ET_INT)
          return t2==ET_INT ? makeobj<ExprEntity_InConstRangeFunc<int_type,int_type,int_type>>(c0,v1,v2)
                            : makeobj<ExprEntity_InConstRangeFunc<int_type,int_type,float_type>>(c0,v1,v2);
        else
          return t2==ET_INT ? makeobj<ExprEntity_InConstRangeFunc<int_type,float_type,int_type>>(c0,v1,v2)
                            : makeobj<ExprEntity_InConstRangeFunc<int_type,float_type,float_type>>(c0,v1,v2);
      } else {
        if (t1==ET_INT)
          return t2==ET_INT ? makeobj<ExprEntity_InConstRangeFunc<float_type,int_type,int_type>>(c0,v1,v2)
                            : makeobj<ExprEntity_InConstRangeFunc<float_type,int_type,float_type>>(c0,v1,v2);
        else
          return t2==ET_INT ? makeobj<ExprEntity_InConstRangeFunc<float_type,float_type,int_type>>(c0,v1,v2)
                            : makeobj<ExprEntity_InConstRangeFunc<float_type,float_type,float_type>>(c0,v1,v2);
      }
    }
  };

  ExprEntityPtr create_inrange(ExprEntityPtr arg0,ExprEntityPtr arg1,ExprEntityPtr arg2)
  {
    auto t0 = arg0->returnType();
    auto t1 = arg1->returnType();
    auto t2 = arg2->returnType();
    if (t0==ET_STRING||t1==ET_STRING||t2==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - inrange function not implemented for strings");
    if (t0==ET_INT) {
      if (t1==ET_INT)
        return t2==ET_INT ? makeobj<ExprEntity_InRangeFunc<int_type,int_type,int_type>>(arg0,arg1,arg2)
                          : makeobj<ExprEntity_InRangeFunc<int_type,int_type,float_type>>(arg0,arg1,arg2);
      else
        return t2==ET_INT ? makeobj<ExprEntity_InRangeFunc<int_type,float_type,int_type>>(arg0,arg1,arg2)
                          : makeobj<ExprEntity_InRangeFunc<int_type,float_type,float_type>>(arg0,arg1,arg2);
    } else {
      if (t1==ET_INT)
        return t2==ET_INT ? makeobj<ExprEntity_InRangeFunc<float_type,int_type,int_type>>(arg0,arg1,arg2)
                          : makeobj<ExprEntity_InRangeFunc<float_type,int_type,float_type>>(arg0,arg1,arg2);
      else
        return t2==ET_INT ? makeobj<ExprEntity_InRangeFunc<float_type,float_type,int_type>>(arg0,arg1,arg2)
                          : makeobj<ExprEntity_InRangeFunc<float_type,float_type,float_type>>(arg0,arg1,arg2);
    }
  }

  template<class type_arg, class type_res, class type_funcarg, class type_funcres, type_funcres the_function(type_funcarg)>
  class ExprEntity_UnaryFunction : public ExprEntity<type_res> {
  public:
    ExprEntity_UnaryFunction(ExprEntityPtr arg, str_type function_name)
      : ExprEntity<type_res>(), m_name("func_"+function_name)
    {
      m_name += '_';
      m_name += exprTypeChar<type_arg>();
      m_name += '2';
      m_name += exprTypeChar<type_res>();
      _ensure_type<type_arg>(arg);
      ExprEntityBase::m_children.push_back(arg);
    }

    virtual str_type name() const { return m_name; }
    virtual ~ExprEntity_UnaryFunction(){}
    virtual type_res evaluate() const {
      return the_function((type_funcarg)_eval<type_arg>(this->ExprEntityBase::child(0)));
    }
  private:
    str_type m_name;
  };

  template<size_t nargs>
  void _ensure_nargs(ExprEntityList& args)
  {
    if (args.size()!=nargs)
      EXPRPARSER_THROW(ParseError,"wrong number of arguments for function");
  }
  void _ensure_1arg(ExprEntityList& args)
  {
    _ensure_nargs<1>(args);
  }

  template<float_type the_function(float_type)>
  ExprEntityPtr create_unary_function_f2f(ExprEntityList& args, const char * name)
  {
    _ensure_1arg(args);
    auto& arg = args.front();
    auto t = arg->returnType();
    if (t==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - numerical function not implemented for strings");
    if (t==ET_INT) {
      return makeobj<ExprEntity_UnaryFunction<int_type,float_type,float_type,float_type,the_function>>(arg,name);
    } else {
      assert_logic(t==ET_FLOAT);
      return makeobj<ExprEntity_UnaryFunction<float_type,float_type,float_type,float_type,the_function>>(arg,name);
    }
  }

  template<float_type the_function_f2f(float_type), float_type the_function_i2f(int_type)>
  ExprEntityPtr create_unary_function_i2f_or_f2f(ExprEntityList& args, const char * name)
  {
    //Use when both f(int)->float and f(float)->float versions exists.
    _ensure_1arg(args);
    auto& arg = args.front();
    auto t = arg->returnType();
    if (t==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - numerical function not implemented for strings");
    if (t==ET_INT) {
      return makeobj<ExprEntity_UnaryFunction<int_type,float_type,int_type,float_type,the_function_i2f>>(arg,name);
    } else {
      assert_logic(t==ET_FLOAT);
      return makeobj<ExprEntity_UnaryFunction<float_type,float_type,float_type,float_type,the_function_f2f>>(arg,name);
    }
  }

  bool isnan_f2b(double x) { return std::isnan(x); }
  bool isinf_f2b(double x) { return std::isinf(x); }

  template<bool the_function(float_type)>
  ExprEntityPtr create_unary_function_f2b(ExprEntityList& args, const char * name)
  {
    _ensure_1arg(args);
    auto& arg = args.front();
    auto t = arg->returnType();
    if (t==ET_STRING)
      EXPRPARSER_THROW(ParseError,"Incompatible types - numerical function not implemented for strings");
    if (t==ET_INT) {
      return makeobj<ExprEntity_UnaryFunction<int_type,int_type,float_type,bool,the_function>>(arg,name);
    } else {
      assert_logic(t==ET_FLOAT);
      return makeobj<ExprEntity_UnaryFunction<float_type,int_type,float_type,bool,the_function>>(arg,name);
    }
  }

  ExprEntityPtr create_std_math_function(const str_type& name, ExprEntityList& args)
  {
    if (name.empty())
      return ExprEntityPtr();
    //Todo: missing hypot, atan2, copysign, fmod, lround
    switch (name[0]) {
    case 'a':
      if (name == "asin")     { return create_unary_function_f2f<std::asin>(args,"asin"); }
      if (name == "acos")     { return create_unary_function_f2f<std::acos>(args,"acos"); }
      if (name == "atan")     { return create_unary_function_f2f<std::atan>(args,"atan"); }
      if (name == "asinh")    { return create_unary_function_f2f<std::asinh>(args,"asinh"); }
      if (name == "acosh")    { return create_unary_function_f2f<std::acosh>(args,"acosh"); }
      if (name == "atanh")    { return create_unary_function_f2f<std::atanh>(args,"atanh"); }
      if (name == "abs")   {  _ensure_1arg(args); return create_function_abs(args.front()); } break;
    case 'b':
      if (name == "bool")  { _ensure_1arg(args); return create_unarybool(args.front()); } break;
    case 'c':
      if (name == "cos")      { return create_unary_function_f2f<std::cos>(args,"cos"); }
      if (name == "cosh")     { return create_unary_function_f2f<std::cosh>(args,"cosh"); }
      if (name == "cbrt")     { return create_unary_function_f2f<std::cbrt>(args,"cbrt"); }
      if (name == "ceil")     { return create_unary_function_f2f<std::ceil>(args,"ceil"); } break;
    case 'e':
      if (name == "erf")      { return create_unary_function_f2f<std::erf>(args,"erf"); }
      if (name == "erfc")     { return create_unary_function_f2f<std::erfc>(args,"erfc"); }
      if (name == "expm1")    { return create_unary_function_f2f<std::expm1>(args,"expm1"); }
      if (name == "exp2")     { return create_unary_function_f2f<std::exp2>(args,"exp2"); }
      if (name == "exp")      { return create_unary_function_f2f<std::exp>(args,"exp"); } break;
    case 'f':
      if (name == "floor")    { return create_unary_function_f2f<std::floor>(args,"floor"); }
      if (name == "fabs")  {  _ensure_1arg(args); return create_function_fabs(args.front()); } break;
    case 'i':
      if (name == "isnan")    { return create_unary_function_f2b<isnan_f2b>(args,"isnan"); }
      if (name == "isinf")    { return create_unary_function_f2b<isinf_f2b>(args,"isinf"); }
      if (name == "isfinite") { return create_unary_function_f2b<std::isfinite>(args,"isfinite"); }
      if (name == "isnormal") { return create_unary_function_f2b<std::isnormal>(args,"isnormal"); }
      if (name == "inrange") { _ensure_nargs<3>(args); return create_inrange(args.at(0),args.at(1),args.at(2)); }
      if (name == "int") { _ensure_1arg(args); return create_typecast(args.front(),ET_INT); }
      if (name == "ipow"){ _ensure_nargs<2>(args); return create_binaryexponentiation_integers(args.front(),args.back()); } break;
    case 'l':
      if (name == "ln"||name == "log") { return create_unary_function_f2f<std::log>(args,"log"); }
      if (name == "log2" )    { return create_unary_function_f2f<std::log2>(args,"log2"); }
      if (name == "log10" )   { return create_unary_function_f2f<std::log10>(args,"log10"); }
      if (name == "log1p" )   { return create_unary_function_f2f<std::log1p>(args,"log1p"); }
      if (name == "lgamma")  { return create_unary_function_i2f_or_f2f<std::lgamma,std::lgamma>(args,"lgamma"); } break;
    case 'm':
      if (name == "min") { return create_function_min(args); }
      if (name == "max") { return create_function_max(args); } break;
    case 'p':
      if (name == "pow") { _ensure_nargs<2>(args); return create_binaryexponentiation(args.front(),args.back()); } break;
    case 'r':
      if (name == "round")    { return create_unary_function_f2f<std::round>(args,"round"); } break;
    case 's':
      if (name == "sin")      { return create_unary_function_f2f<std::sin>(args,"sin"); }
      if (name == "sinh")     { return create_unary_function_f2f<std::sinh>(args,"sinh"); }
      if (name == "sqrt")     { return create_unary_function_f2f<std::sqrt>(args,"sqrt"); }
      if (name == "str" ||name == "string") { _ensure_1arg(args); return create_typecast(args.front(),ET_STRING); }
      if (name == "sum") { return create_function_sum(args); } break;
    case 't':
      if (name == "tan")      { return create_unary_function_f2f<std::tan>(args,"tan"); }
      if (name == "tanh")     { return create_unary_function_f2f<std::tanh>(args,"tanh"); }
      if (name == "trunc")    { return create_unary_function_f2f<std::trunc>(args,"trunc"); }
      if (name == "tgamma")  { return create_unary_function_i2f_or_f2f<std::tgamma,std::tgamma>(args,"lgamma"); } break;
    }
    //two aliases, different first char:
    if (name == "double" || name == "float") { _ensure_1arg(args); return create_typecast(args.front(),ET_FLOAT); }
    //not found:
    return ExprEntityPtr();
  }

  bool classify_std_math_prefix_operator(const str_type& name, unsigned & precedence)
  {
    //C++-like, see comments in classify_std_math_binary_operator.
    const unsigned prec_negation = 170;
    if (name.size()==1) {
      switch (name[0]) {
      case '-' : precedence = 170; return true;//unary minus
      case '+' : precedence = 170; return true;//unary plus
      case '~' : precedence = 170; return true;//bitwise not
      case '!' : precedence = prec_negation; return true;//negation
      }
    } else if (name=="not") {
      //negation
      precedence = prec_negation;
      return true;
    }
    precedence = 0;
    return false;
  }

  bool classify_std_math_binary_operator(const str_type& name, unsigned & precedence, bool& is_right_associative)
  {
    //Implement operators like in C/C++ with a few exceptions (^ or ** is
    //exponentiation operator, and bitwise or ^ operator from C++ is renamed "XOR")

    is_right_associative = false;//all of the following except exponentiation
                                 //are left-associative.

    const unsigned prec_exp = 170;//NB: Put 165 here to mimic excels bad
                                  //precedence order (thus making -3^2 = 9,
                                  //rather than -3^2 = -9 like it does in most
                                  //programming languages).
    const unsigned prec_and = 70;

    if (name.size()==1) {
      switch (name[0]) {
      case '*' : precedence = 150; return true;//multiplication
      case '/' : precedence = 150; return true;//division
      case '%' : precedence = 150; return true;//modulo
      case '+' : precedence = 140; return true;//addition
      case '-' : precedence = 140; return true;//subtraction
      case '<' : precedence = 120; return true;//less-than
      case '>' : precedence = 120; return true;//greater-than
      case '&' : precedence = 100; return true;//bitwise AND
      case '|' : precedence = 80; return true;//bitwise OR
      case '^' : //exponentiation
        is_right_associative = true;
        precedence = prec_exp;
        return true;
      }
    } else if (name.size()==2) {
      if (name == "<<") { precedence = 130; return true; }//bitwise left shift
      if (name == ">>") { precedence = 130; return true; }//bitwise right shift
      if (name == "<=") { precedence = 120; return true; }//less-than-or-equal
      if (name == ">=") { precedence = 120; return true; }//greater-than-or-equal
      if (name == "==") { precedence = 110; return true; }//equal-to
      if (name == "!=") { precedence = 110; return true; }//not-equal-to
      if (name == "&&") { precedence = prec_and; return true; }//logical and
      if (name == "||" || name=="or") { precedence = 60; return true; }//logical or
      if (name == "**") {
        //same as '^' above.
        is_right_associative = true;
        precedence = prec_exp; return true;
      }
    }
    if (name == "and") {
      precedence = prec_and;
      return true;
    }
    if (name == "xor") {
      precedence = 90;
      return true;//bitwise XOR (we use ^ for exponentiation)
    }
    precedence = 0;
    return false;
  }

  ExprEntityPtr create_std_math_prefix_operator(const str_type& name, ExprEntityPtr arg)
  {
    if (name.size()==1) {
      switch (name[0]) {
      case '-': return create_unaryminus(arg);
      case '+': return create_unaryplus(arg);
      case '!': return create_booleannot(arg);
      case '~' : return create_bitwise_not(arg);
      }
    } else if (name=="not") {
      return create_booleannot(arg);
    }
    return 0;
  }

  ExprEntityPtr create_std_math_binary_operator(const str_type& name, ExprEntityPtr arg1, ExprEntityPtr arg2)
  {
    if (name.size()==1) {
      switch (name[0]) {
      case '+': return create_binaryaddition(arg1,arg2);
      case '-': return create_binarysubtraction(arg1,arg2);
      case '/': return create_binarydivision(arg1,arg2);
      case '*': return create_binarymultiplication(arg1,arg2);
      case '^': return create_binaryexponentiation(arg1,arg2);
      case '%': return create_binarymodulo(arg1,arg2);
      case '<': return create_cmp_lt(arg1,arg2);
      case '>': return create_cmp_gt(arg1,arg2);
      case '&': return create_bitwise_and(arg1,arg2);
      case '|': return create_bitwise_or(arg1,arg2);
      }
    } else if (name=="**") {
      return create_binaryexponentiation(arg1,arg2);
    } else if (name=="&&"||name=="and") {
      return create_booleanand(arg1,arg2);
    } else if (name=="||"||name=="or") {
      return create_booleanor(arg1,arg2);
    } else if (name=="==") {
      return create_cmp_equal(arg1,arg2);
    } else if (name=="!=") {
      return create_cmp_notequal(arg1,arg2);
    } else if (name=="<=") {
      return create_cmp_le(arg1,arg2);
    } else if (name==">=") {
      return create_cmp_ge(arg1,arg2);
    } else if (name=="xor") {
      return create_bitwise_xor(arg1,arg2);
    } else if (name=="<<") {
      return create_bitwise_lshift(arg1,arg2);
    } else if (name==">>") {
      return create_bitwise_rshift(arg1,arg2);
    }
    return 0;
  }

}
