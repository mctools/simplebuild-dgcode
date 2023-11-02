#include "ExprParser/ASTNode.hh"

namespace ExprParser {

  bool ExprEntityBase::isConstant() const
  {
    //This default implementation does not handle case where the result might be
    //constant even if not all children are (i.e. "var && false" is always false
    //- and if evaluation of 'var' does not have sideeffects, it could be
    //ignored - "false && var" would not care about var having sideeffects).
    for (auto& c : m_children) {
      if (!c->isConstant())
        return false;
    }
    return true;
  }

  void ExprEntityBase::optimiseChildren()
  {
    for (size_t i = 0; i < m_children.size(); ++i) {
      auto opt = m_children[i]->optimisedVersion();
      if (opt) {
        if (m_children[i]->returnType()!=opt->returnType())
          EXPRPARSER_THROW(LogicError,"node changes return type upon optimisation");
        m_children[i] = opt;
      }
    }
  }

  void optimise(ExprEntityPtr& p)
  {
    auto opt = p->optimisedVersion();
    if (opt)
      p = opt;
  }

  ExprEntityPtr create_constant(float_type val) { return makeobj<ExprEntityConstantValue<float_type>>(val); }
  ExprEntityPtr create_constant(int_type val) { return makeobj<ExprEntityConstantValue<int_type>>(val); }
  ExprEntityPtr create_constant(const str_type& val) { return makeobj<ExprEntityConstantValue<str_type>>(val); }

  template<class TValue>
  class ExprEntity_UnaryBool : public ExprEntity<int_type> {
  public:
    virtual str_type name() const {
      str_type s("UnaryBool_");
      s += exprTypeChar<TValue>();
      return s;
    }
    ExprEntity_UnaryBool(ExprEntityPtr arg)
      : ExprEntity<int_type>()
    {
      _ensure_type<TValue>(arg);
      ExprEntityBase::m_children.push_back(arg);
    }
    virtual ~ExprEntity_UnaryBool(){}
    virtual int_type evaluate() const {
      return _boolify(_eval<TValue>(this->ExprEntityBase::child(0)));
    }
  };



  ExprEntityPtr create_unarybool(ExprEntityPtr arg)
  {
    auto t = arg->returnType();
    if (t==ET_STRING)
      return makeobj<ExprEntity_UnaryBool<str_type>>(arg);
    if (t==ET_INT)
      return makeobj<ExprEntity_UnaryBool<int_type>>(arg);
    _ensure_type<float_type>(arg);
    return makeobj<ExprEntity_UnaryBool<float_type>>(arg);
  }

  template<class type_arg, class type_res>
  class ExprEntity_TypeCast : public ExprEntity<type_res> {
  public:
    virtual str_type name() const {
      str_type s("TypeCast_");
      s += exprTypeChar<type_arg>();
      s += '2';
      s += exprTypeChar<type_res>();
      return s;
    }
    ExprEntity_TypeCast(ExprEntityPtr arg)
      : ExprEntity<type_res>()
    {
      _ensure_type<type_arg>(arg);
      ExprEntityBase::m_children.push_back(arg);
    }
    virtual ~ExprEntity_TypeCast(){}
    virtual type_res evaluate() const {
      return _eval<type_arg>(this->ExprEntityBase::child(0));
    }
  };

  template<> inline int_type ExprEntity_TypeCast<str_type,int_type>::evaluate() const
  {
    //TODO: This will accept "1234hello", we should use the second parameter to
    //check all chars were converted!
    //
    str_type v = _eval<str_type>(this->ExprEntityBase::child(0));
    int_type res;
    std::size_t nprocessed(0);
    try {
      if (sizeof(int_type)==sizeof(long))
        res = std::stol( v, &nprocessed, 0/*autodetect base*/ );
      else
        res = std::stoll( v, &nprocessed, 0/*autodetect base*/ );
    } catch ( std::logic_error& ) {
      nprocessed = 0;
    }
    if (!nprocessed || nprocessed < v.size())
      EXPRPARSER_THROW2(DomainError,"Can not cast string to integer constant : \"" <<v<<"\"");
    return res;
  }
  template<> inline float_type ExprEntity_TypeCast<str_type,float_type>::evaluate() const
  {
    //TODO: This will accept "1.234hello", we should use the second parameter to
    //check all chars were converted!
    str_type v = _eval<str_type>(this->ExprEntityBase::child(0));
    float_type res;
    std::size_t nprocessed(0);
    try {
      res = std::stod( v, &nprocessed );
    } catch ( std::invalid_argument& ) {
      nprocessed = 0;
    }
    if (!nprocessed || nprocessed < v.size())
      EXPRPARSER_THROW2(DomainError,"Can not cast string to floating point constant : \"" <<v<<"\"");
    return res;
  }
  template<> inline str_type ExprEntity_TypeCast<int_type,str_type>::evaluate() const {
    return std::to_string(_eval<int_type>(this->ExprEntityBase::child(0)));
  }
  template<> inline str_type ExprEntity_TypeCast<float_type,str_type>::evaluate() const {
    return std::to_string(_eval<float_type>(this->ExprEntityBase::child(0)));
  }

  template<class target_type>
  ExprEntityPtr create_typecast(ExprEntityPtr arg)
  {
    auto t = arg->returnType();
    if (t==exprType<target_type>())
      return arg;//already correct type
    if (t==ET_STRING)
      return makeobj<ExprEntity_TypeCast<str_type,target_type>>(arg);
    if (t==ET_INT)
      return makeobj<ExprEntity_TypeCast<int_type,target_type>>(arg);
    _ensure_type<float_type>(arg);
    return makeobj<ExprEntity_TypeCast<float_type,target_type>>(arg);
  }

  ExprEntityPtr create_typecast(ExprEntityPtr arg, ExprType target_type)
  {
    if (arg->returnType()==target_type)
      return arg;//already correct type
    if (target_type==ET_STRING) return create_typecast<str_type>(arg);
    else if (target_type==ET_INT) return create_typecast<int_type>(arg);
    assert_logic(target_type==ET_FLOAT);
    return create_typecast<float_type>(arg);
  }

  template<>
  void Evaluator<bool>::setArg(ExprEntityPtr p)
  {
    if (!(m_p=p))
      return;
    //Normally expressions passed here would involve comparison operators and
    //thus return integers already. Thus we optimise for those, and simply add
    //a boolean cast (to int 1l or 0l) to other types.
    if (m_p->returnType()!=ET_INT)
      m_p = create_unarybool(m_p);
    _ensure_type<int_type>(m_p);
  }

  template<>
  void Evaluator<str_type>::setArg(ExprEntityPtr p)
  {
    if (!(m_p=p))
      return;
    //No implicit cast to/from strings:
    if (m_p->returnType()!=ET_STRING)
      EXPRPARSER_THROW(DomainError,"provided expression does not result in a string object");
    _ensure_type<str_type>(m_p);
  }

  template<>
  void Evaluator<int_type>::setArg(ExprEntityPtr p)
  {
    if (!(m_p=p))
      return;
    //No implicit cast to/from strings and no implicit demotion of floats to ints:
    if (m_p->returnType()!=ET_INT)
      EXPRPARSER_THROW(DomainError,"provided expression does not result in an integer");
    _ensure_type<int_type>(m_p);
  }

  template<>
  void Evaluator<float_type>::setArg(ExprEntityPtr p)
  {
    if (!(m_p=p))
      return;
    //No implicit cast to/from strings:
    if (m_p->returnType()==ET_STRING)
      EXPRPARSER_THROW(DomainError,"provided expression results in a string object rather than a number");
    //Implicitly promote ints to floats:
    if (m_p->returnType()==ET_INT)
      m_p = create_typecast(m_p, ET_FLOAT);
    _ensure_type<float_type>(m_p);
  }

  // Explicit template instantiation
  template class Evaluator<bool>;
  template class Evaluator<int_type>;
  template class Evaluator<float_type>;
  template class Evaluator<str_type>;

}
