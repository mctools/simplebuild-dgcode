#ifndef ExprParser_ASTNode_hh
#define ExprParser_ASTNode_hh

//Node interface for abstract syntax tree representing (possibly optimised
//versions of) expressions and allowing their evaluation.
//
//All nodes inherit from ExprEntityBase who keeps a vector of child nodes (this
//vector is the only place nodes are allowed to keep references to other
//nodes!). Our expressions support tree types: float_type (double), int_type (64
//bit signed int) and a str_type (std::string). Rather than inheriting directly
//from ExprEntityBase, nodes must inherit from one of ExprEntity<float_type>,
//ExprEntity<int_type> or ExprEntity<str_type> as appropriate for their type and
//implement the relevant evaluate() method.

//todo: check includes
#include "ExprParser/Types.hh"
#include "ExprParser/Exception.hh"
#include <vector>
#include <memory>
#include <stdexcept>

namespace ExprParser {

  //Type info:

  class ExprEntityBase;
  typedef std::shared_ptr<ExprEntityBase> ExprEntityPtr;
  typedef std::vector<ExprEntityPtr> ExprEntityList;

  //Base class for all nodes (only ExprEntity<..> will inherit directly from it):

  class ExprEntityBase {
  public:
    ExprEntityBase(){}
    virtual ~ExprEntityBase(){}

    virtual bool isConstant() const;//Default implementation returns true when
                                    //all children are constant.

    virtual ExprType returnType() const = 0;
    virtual str_type name() const = 0;

    //optimises child nodes and returns non-null instance if a more optimal
    //version will be available (if so, the holder can use it to replace
    //this). Implementation is allowed to optimise child nodes as desired when
    //this is called:
    virtual ExprEntityPtr optimisedVersion() { return ExprEntityPtr(); }
    const ExprEntityList& children() const { return m_children; }
  protected:
    void optimiseChildren();
    virtual ExprEntityPtr specialOptimisedVersion() const
    {
      //Reimplement if optimised versions might exist even if not all
      //children are constants (like "a + 0" -> "a").
      return ExprEntityPtr();
    }
    const ExprEntityPtr& child(size_t i) const;
    ExprEntityPtr& child(size_t i);
    ExprEntityList m_children;//child nodes (i.e. operands or arguments)
  };

  template<class TValue>
  class ExprEntity : public ExprEntityBase {
    //TODO: Specialisation possible for str which returns by ref in evaluate??
  public:
    ExprEntity() : ExprEntityBase() {}
    virtual ~ExprEntity(){}
    virtual ExprType returnType() const final;
    virtual TValue evaluate() const = 0;
    virtual ExprEntityPtr optimisedVersion();
  };

  template<class TValue>
  class ExprEntityConstantValue final : public ExprEntity<TValue> {
    //Class representing a constant value. This class is final so we can be sure
    //a dynamic_cast to ExprEntityConstantValue<TValue>* will only be successful
    //for this exact class instance (useful when optimising the node tree).
  public:
    virtual str_type name() const;
    explicit ExprEntityConstantValue(TValue val);
    virtual ~ExprEntityConstantValue(){}
    virtual bool isConstant() const { return true; }
    virtual TValue evaluate() const { return m_val; }
  private:
    TValue m_val;
  };

  //Create all instances of expression entity objects with this to ensure
  //exception safety and reasonable compilation times and object sizes:
  template<typename T, typename ...Arg>
  inline ExprEntityPtr makeobj(Arg && ... arg);

  //Easily create a constant value:
  ExprEntityPtr create_constant(float_type val);
  ExprEntityPtr create_constant(int_type val);
  ExprEntityPtr create_constant(const str_type& val);

  //Optimises p (possibly replacing the object with a more optimal version):
  void optimise(ExprEntityPtr& p);

  //Various utilities for evaluating types and values of entities:
  //Todo: Remove leading "_" in these, since they are public:?
  template<class TValue> void _ensure_type(const ExprEntityPtr& p);//throws if not correct type
  template<class TValue> void _ensure_type_dbg(const ExprEntityPtr& p);//same, but does nothing in non-dbg builds
  template<class TValue> TValue _eval(const ExprEntityPtr& p);//calls _ensure_type_dbg and then evaluates.
  template<class type_val> int_type _boolify(const type_val& v) { return v ? 1l : 0l; }
  template<> inline int_type _boolify(const str_type& v) { return v.empty() ? 0l : 1l; }
  template<class type_val> bool _is_true(const type_val& v) { return (bool)v; }
  template<> inline bool _is_true(const str_type& v) { return !v.empty(); }
  template<class val_type> constexpr ExprType exprType();
  template<class val_type> constexpr char exprTypeChar();//'i', 'f' or 's'
  ExprEntityPtr create_unarybool(ExprEntityPtr arg);
  ExprEntityPtr create_typecast(ExprEntityPtr arg, ExprType target_type);

  //Helper class used to finish up (by type-casts, if needed) and evaluate
  //expressions to ensure their evaluation type will be correctly TValue, which
  //can be either bool or one of the float_type, str_type, int_type or
  //bool. Attempting to mix float/int with strings will raise exceptions (since
  //they should not convert implicitly to each other):

  template<class TValue>
  class Evaluator {
  public:
    Evaluator(ExprEntityPtr p = 0);
    ~Evaluator(){}
    void setArg(ExprEntityPtr p);
    TValue operator()() const;//undefined behaviour if p==0
    ExprEntityPtr arg() { return m_p; }
    const ExprEntityPtr arg() const { return m_p; }
    bool isConstant() const { return m_p && m_p->isConstant(); }
  private:
    ExprEntityPtr m_p;
  };

  template<class TValue>
  inline TValue Evaluator<TValue>::operator()() const { return _eval<TValue>(m_p); }
  template<>
  inline bool Evaluator<bool>::operator()() const { return _eval<int_type>(m_p); }
}

#include "ExprParser/ASTNode.icc"

#endif
