#include "ExprParser/ASTBuilder.hh"
#include "ExprParser/ASTStdMath.hh"
#include "ExprParser/Exception.hh"
#include <stack>

namespace ExprParser {

  //Shunting Yard Algorithm* with extensions to not just binary infix operators
  //but also unary operators and function calls of fixed or variadic arity
  //
  //http://wcipeg.com/wiki/Shunting_yard_algorithm#Extensions
  //http://www.reedbeta.com/blog/2011/12/11/the-shunting-yard-algorithm/
  //
  //*: Dijkstra, E. W. (1961). ALGOL-60 Translation. Retrieved from http://www.cs.utexas.edu/~EWD/MCReps/MR35.PDF
  //
  //Note that the same identifier can be used as both binary and unary operator
  //(e.g. '-', '+' and '!'), but apart from that, identifiers can not mix between
  //operators, constants and functions.

  struct ASTBuilder::Imp {
    struct OperatorInfo;
    static ExprEntityPtr littValToken2Entity(const TokenPtr& tk);
    static ExprEntityPtr op2entity(const ASTBuilder* THIS,
                                   const OperatorInfo& op,
                                   std::stack<ExprEntityPtr>& operand_stack);
    static void applyops( const ASTBuilder* THIS,
                          std::stack<OperatorInfo>& operator_stack,
                          std::stack<ExprEntityPtr>& operand_stack,
                          unsigned precedence_limit, bool include_those_at_limit);
  };

  struct ASTBuilder::Imp::OperatorInfo {

    enum OperatorType { OT_LeftParen, OT_FUNC, OT_UNARY_PREFIX, OT_UNARY_POSTFIX, OT_BINARY };

    OperatorInfo(const std::string& n, unsigned pr, OperatorType ot)
      : name(n), optype(ot), precedence(pr)
    {
      //represents an operator
      assert_logic( !name.empty() && pr>0);

    };
    OperatorInfo( const std::string& n )
      : name(n), optype(n=="("?OT_LeftParen:OT_FUNC), precedence(0)
    {
      //represents a named function or a left-parenthesis (when n=="(")
      assert_logic( !name.empty() );
      if (optype==OT_FUNC)
        function_args = std::make_shared<ExprEntityList>();
    };
    bool isFunction() { return optype==OT_FUNC; }
    const std::string& functionName() { return name; }
    bool isLeftParen() { return optype==OT_LeftParen; }
    bool isOperator() { return optype==OT_BINARY||optype==OT_UNARY_PREFIX||optype==OT_UNARY_POSTFIX; }
    size_t arity() const { return optype==OT_BINARY ? 2 : ((optype==OT_UNARY_PREFIX||optype==OT_UNARY_POSTFIX)?1:0); }
    const std::string name;
    const OperatorType optype;
    const unsigned precedence;
    std::shared_ptr<ExprEntityList> function_args;
  };

  ExprEntityPtr ASTBuilder::Imp::littValToken2Entity(const TokenPtr& tk)
  {
    assert_logic(tk->isLitteralValue());
    if (tk->isLitteralValueFloat()) {
      float_type v;
      std::size_t nprocessed(0);
      try {
        v = std::stod(tk->content(),&nprocessed);
      } catch ( std::logic_error& ) {
        nprocessed = 0;
      }
      if (!nprocessed || nprocessed < tk->content().size())
        EXPRPARSER_THROW2(LogicError,"Could not convert token to floating point constant : " <<tk->content());
      return create_constant(v);
    } else if (tk->isLitteralValueInteger()) {
      int_type val;
      std::size_t nprocessed(0);
      try {
        if (sizeof(int_type)==sizeof(long))
          val = std::stol( tk->content(), &nprocessed, 0/*autodetect base*/ );
        else
          val = std::stoll( tk->content(), &nprocessed, 0/*autodetect base*/ );
      } catch ( std::logic_error& ) {
        nprocessed = 0;
      }
      if (!nprocessed || nprocessed < tk->content().size())
        EXPRPARSER_THROW2(LogicError,"Could not convert token to integer constant : " <<tk->content());

      return create_constant(val);
    } else {
      assert_logic(tk->isLitteralValueString());
      return makeobj<ExprEntityConstantValue<str_type>>(tk->content());
    }
  }

  ExprEntityPtr ASTBuilder::Imp::op2entity(const ASTBuilder* THIS,const OperatorInfo& op, std::stack<ExprEntityPtr>& operand_stack)
  {
    if (op.arity() > operand_stack.size()) {
      //todo: can this actually happen? Can we trigger this? ??
      EXPRPARSER_THROW(ParseError,"not enough operands for operator");
    }
    ExprEntityList args;
    for (unsigned i = 0; i < op.arity(); ++i) {
      args.push_back(operand_stack.top());
      operand_stack.pop();
    }

    assert_logic(args.size()==op.arity());

    ExprEntityPtr result = 0;
    if ( op.optype == OperatorInfo::OT_UNARY_PREFIX ) {
      result = THIS->createPrefixOperator(op.name,args.back());
    } else if ( op.optype == OperatorInfo::OT_UNARY_POSTFIX ) {
      result = THIS->createPostfixOperator(op.name,args.back());
    } else if ( op.optype == OperatorInfo::OT_BINARY ) {
      result = THIS->createBinaryOperator(op.name,args.back(),args.front());//Todo: right order of args?
    } else {
      EXPRPARSER_THROW(LogicError,"unexpected optype");
    }
    if (!result)
      EXPRPARSER_THROW2(LogicError,"implementation error - could classify but not create operator : "<<op.name);
    return result;
  }


  ExprEntityPtr ASTBuilder::buildTree(const TokenList& tokens) const
  {
    std::stack<ExprEntityPtr> operand_stack;
    std::stack<Imp::OperatorInfo> operator_stack;

    str_type impliedmult_symb;
    bool impliedmult_right_assoc;
    unsigned impliedmult_prec = classifyImpliedMultiplication(impliedmult_symb,impliedmult_right_assoc);

    bool prefix = true;//whether an operator at location must be prefix/fct or postfix/infix

    for(size_t i = 0; i < tokens.size(); ++i) {
      auto& tk = tokens[i];
      auto& tk_prev = i>0 ? tokens[i-1] : TokenPtr();
      auto& tk_next = i+1<tokens.size() ? tokens[i+1] : TokenPtr();

      if (tk->isLeftParen()) {
        if (prefix) {
          if ( i+1<tokens.size() && tokens[i+1]->isRightParen() )
            EXPRPARSER_THROW(ParseError,"empty parenthesis");
          operator_stack.push(Imp::OperatorInfo("("));
        } else {
          //This must be the opening parenthesis of a function call, but those
          //we should have already skipped past. Hence, we are left with stuff like "5()":
          if ( tk_prev ) {
            EXPRPARSER_THROW2(ParseError,"forbidden function call parenthesis after "<<tk_prev->content());
          } else {
            EXPRPARSER_THROW(LogicError,"can not classify forbidden function call parenthesis");
          }
        }
        prefix = true;
        continue;
      }
      if (tk->isComma()) {
        if (prefix)
          EXPRPARSER_THROW(ParseError,"unexpected comma");
        if ( i+1<tokens.size() && (tokens[i+1]->isRightParen()||tokens[i+1]->isComma()) )
          EXPRPARSER_THROW(ParseError,"missing argument after comma");
        Imp::applyops(this,operator_stack, operand_stack, 0, false);
        if (operator_stack.empty()||!operator_stack.top().isFunction())
          EXPRPARSER_THROW(ParseError,"comma used outside function argument list");
        if (operand_stack.empty())
          EXPRPARSER_THROW(ParseError,"missing argument preceeding comma in function call");//can't happen? LogicError instead?
        operator_stack.top().function_args->push_back(operand_stack.top());
        operand_stack.pop();
        prefix = true;
        continue;
      }
      if (tk->isLitteralValue()) {
        if (!prefix)
          EXPRPARSER_THROW2(ParseError,"unexpected value : "<<tk->content());//litteral value appearing in postfix position
        prefix = false;
        operand_stack.push(Imp::littValToken2Entity(tk));
        continue;
      }
      if (tk->isRightParen()) {
        prefix = false;
        //Closing grouping-parenthesis. Pop and apply any operators on the stack
        //until we reach a left-parenthesis or a function call operator (which
        //we finish up by popping).
        Imp::applyops(this,operator_stack, operand_stack, 0, false);
        if ( operator_stack.empty() || !( operator_stack.top().isLeftParen() || operator_stack.top().isFunction() ) )
          EXPRPARSER_THROW(ParseError,"mismatched parenthesis");//closing paren which was never opened
        if ( operator_stack.top().isFunction() ) {
          //Closing parenthesis is actually a function call and the operand at
          //the top of the operand stack is a function argument.
          if (operand_stack.empty()) {
            //must be at least one more argument to flush to function (we
            //checked for and dealt with 0-argument functions already when
            //encountering the start of the function call, and syntax errors
            //like "sin(3,)" were checked when encountering the comma).
            EXPRPARSER_THROW(LogicError,"could not find argument for function call");
          }
          operator_stack.top().function_args->push_back(operand_stack.top());
          operand_stack.pop();
          auto f = createFunction(operator_stack.top().name,
                                  *(operator_stack.top().function_args));
          if (!f)
            EXPRPARSER_THROW2(ParseError,"unknown function : " << operator_stack.top().name
                              << " [" << operator_stack.top().function_args->size() <<" arguments]");
          operand_stack.push(f);
        }
        operator_stack.pop();
        continue;
      }
      assert_logic(tk->isIdentifier());

      bool prev_number = tk_prev && ( tk_prev->isLitteralValueInteger() || tk_prev->isLitteralValueFloat() );

      if ( ( prefix || (impliedmult_prec && prev_number) )
           && tk->isIdentifierNamed() && tk_next && tk_next->isLeftParen()) {
        //Apparently this is a function call:
        if (!prefix) {
          //implied multiplication
          assert_logic(impliedmult_prec && prev_number);
          Imp::applyops(this, operator_stack, operand_stack, impliedmult_prec, !impliedmult_right_assoc);
          operator_stack.push(Imp::OperatorInfo(impliedmult_symb, impliedmult_prec, Imp::OperatorInfo::OT_BINARY));
        }
        if ( i+2<tokens.size() && tokens[i+2]->isRightParen() ) {
          //with no arguments, e.g. like time().
          ExprEntityList noargs;
          auto f = createFunction(tk->content(),noargs);
          if (!f)
            EXPRPARSER_THROW2(ParseError,"unknown function : " << tk->content() << " [ 0 arguments]");
          operand_stack.push(f);
          i += 2;//consume both parenthesis in function call
        } else {
          operator_stack.push(Imp::OperatorInfo(tk->content()));//function
          ++i;//consume opening parenthesis of function call as well
          prefix=true;
        }
        continue;
      }

      if ( prefix || (impliedmult_prec && prev_number) ) {
        auto val = createValue(tk->content());
        if (val) {
          if (!prefix) {
            //implied multiplication
            assert_logic(impliedmult_prec && prev_number);
            Imp::applyops(this, operator_stack, operand_stack, impliedmult_prec, !impliedmult_right_assoc);
            operator_stack.push(Imp::OperatorInfo(impliedmult_symb, impliedmult_prec, Imp::OperatorInfo::OT_BINARY));
          }
          operand_stack.push(val);
          prefix = false;
          continue;
        }
      }


      //Ok, must be operator if anything!

      if (!tk->isIdentifier())
        EXPRPARSER_THROW2(LogicError, "expected identifier but got : " << tk->content())

      const char * errmsg = tk->isIdentifierNamed() ? "Unknown or badly placed identifier : "
                                                    : "Unknown or badly placed symbol : ";

      if (prefix) {
        //This can be an unary prefix operator
        unsigned prec = this->classifyPrefixOperator(tk->content());
        if (!prec)
          EXPRPARSER_THROW2(ParseError, errmsg << tk->content())
        Imp::applyops(this, operator_stack, operand_stack, prec, false/*right associative*/);
        operator_stack.push(Imp::OperatorInfo(tk->content(), prec, Imp::OperatorInfo::OT_UNARY_PREFIX));
      } else {
        //This can be an unary postfix operator or a binary operator
        auto optype = Imp::OperatorInfo::OT_BINARY;
        bool right_assoc(false);
        unsigned prec = classifyBinaryOperator(tk->content(),right_assoc);
        if ( !prec ) {
          optype = Imp::OperatorInfo::OT_UNARY_POSTFIX;
          prec = classifyPostfixOperator(tk->content());
          if ( !prec )
            EXPRPARSER_THROW2(ParseError, errmsg << tk->content());
        }
        Imp::applyops(this, operator_stack, operand_stack, prec, !right_assoc);
        operator_stack.push(Imp::OperatorInfo(tk->content(), prec, optype));
      }
      prefix = true;
    }

    Imp::applyops(this,operator_stack,operand_stack,0,true);

    if (!operator_stack.empty())
      EXPRPARSER_THROW(LogicError,"Non-empty operator stack");

    if (operand_stack.empty())
      EXPRPARSER_THROW(ParseError,"syntax error : empty expression");

    if (operand_stack.size()!=1)
      EXPRPARSER_THROW(LogicError,"Failed to catch syntax error on multiple consequetive values:");//parse error, but should have been caught earlier.

    auto res = operand_stack.top();
    return res;
  }

  void ASTBuilder::Imp::applyops( const ASTBuilder * THIS,
                                  std::stack<Imp::OperatorInfo>& operator_stack, std::stack<ExprEntityPtr>& operand_stack,
                                  unsigned precedence_limit, bool include_those_at_limit)
  {
    //apply all operators waiting on stack which are of higher precedence than a given limit
    //
    //precedence limit of zero indicates evaluation at end of expression or end
    //of parenthesis (grouping or function-call).
    while(!operator_stack.empty()) {
      if (operator_stack.top().isFunction()) {
        if (precedence_limit>0||!include_those_at_limit)
          return;
        EXPRPARSER_THROW2(ParseError,"function call never closed in function : "<<(operator_stack.top().functionName()));
      }
      if (operator_stack.top().isLeftParen()) {
        if (precedence_limit>0||!include_those_at_limit)
          return;
        EXPRPARSER_THROW(ParseError,"parenthesis never closed");
      }
      if ( operator_stack.top().precedence<precedence_limit
           || (!include_those_at_limit && operator_stack.top().precedence==precedence_limit) )
        return;
      if (operand_stack.size()<operator_stack.top().arity())
        EXPRPARSER_THROW2(ParseError,"missing arguments for operator : "<<operator_stack.top().name);
      operand_stack.push(Imp::op2entity(THIS,operator_stack.top(),operand_stack));//todo: is _op2entity only needed here?
      operator_stack.pop();
    }
  }


  ExprEntityPtr ASTBuilder::createValue(const str_type& name) const
  {
    return create_std_math_constant(name);
  }

  ExprEntityPtr ASTBuilder::createFunction(const str_type& n, ExprEntityList& args) const
  {
    return create_std_math_function(n,args);
  }

  unsigned ASTBuilder::classifyPrefixOperator(const str_type& identifier) const
  {
    unsigned precedence;
    return classify_std_math_prefix_operator(identifier,precedence) ? precedence : 0;
  }

  unsigned ASTBuilder::classifyBinaryOperator(const str_type& identifier, bool& is_right_associative) const
  {
    unsigned precedence;
    return classify_std_math_binary_operator(identifier,precedence,is_right_associative) ? precedence : 0;
  }

  unsigned ASTBuilder::classifyPostfixOperator(const str_type& ) const
  {
    //We don't provide any postfix operators by default.
    return 0;
  }

  ExprEntityPtr ASTBuilder::createPrefixOperator(const str_type& identifier, ExprEntityPtr arg) const
  {
    return create_std_math_prefix_operator(identifier,arg);
  }

  ExprEntityPtr ASTBuilder::createBinaryOperator(const str_type& identifier, ExprEntityPtr arg1, ExprEntityPtr arg2) const
  {
    return create_std_math_binary_operator(identifier,arg1,arg2);
  }

  ExprEntityPtr ASTBuilder::createPostfixOperator(const str_type&, ExprEntityPtr) const
  {
    //We don't provide any postfix operators by default.
    return ExprEntityPtr();
  }

  unsigned ASTBuilder::classifyImpliedMultiplication(str_type& symbol, bool& is_right_associative) const
  {
    symbol = "*";
    unsigned prec_explicit_mult = classifyBinaryOperator(symbol,is_right_associative);
    return prec_explicit_mult ? prec_explicit_mult + 1 : 0;
  }

}
