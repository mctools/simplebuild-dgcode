#ifndef ExprParser_Exception_hh
#define ExprParser_Exception_hh

#include <stdexcept>
#include <sstream>

namespace ExprParser {

  //Base class for all ExprParser exceptions.
  //
  //NB: The destructors of all exception classes are defined in Exception.cc
  //(rather than inline), so that their type information is only present in the
  //ExprParser library. Otherwise each module using them (e.g. Python modules,
  //which are loaded with RTLD_LOCAL) would get its own copy, and on some
  //platforms (e.g. Intel macOS, where libc++ compares type information by
  //address) an exception thrown in one module could not be caught by type in
  //another module.

  class Exception {
  public:
    Exception() {}
    virtual ~Exception() throw();
    virtual const char * epType() const = 0;
    virtual const char * epWhat() const = 0;
  };

  //Base class for user/input related errors (syntax errors, division with zero,
  //etc.). Normally this can be caught and the error presented to the user,
  //while not inadvertently catching other errors (like errors in implementation
  //logic):

  class InputError : public Exception {
  public:
    using Exception::Exception;
    virtual ~InputError() throw();
  };

  //Exception thrown if input expressions contains errors (for instance syntax
  //errors usage of undefined variables):

  class ParseError : public std::runtime_error, public InputError {
  public:
    using std::runtime_error::runtime_error;
    virtual ~ParseError() throw();
    const char * epType() const { return "ParseError"; }
    virtual const char * epWhat() const { return what(); }
  };

  //Exception thrown when domain errors (like division by zero) is encountered
  //in expressions. Note, that not all such problems are guaranteed to result in
  //this exception being thrown, since for efficiency nodes are allowed to
  //simply assume correct domains and pass numbers directly to e.g. inbuilt C++
  //operators or libmath functions).

  class DomainError : public std::domain_error, public InputError {
  public:
    using std::domain_error::domain_error;
    virtual ~DomainError() throw();
    const char * epType() const { return "DomainError"; }
    virtual const char * epWhat() const { return what(); }
  };

  //Exception thrown due to unexpected logic errors (thus usually indicating an
  //implementation error in the expression parser code itself):

  class LogicError : public std::logic_error, public Exception {
  public:
    using std::logic_error::logic_error;
    virtual ~LogicError() throw();
    const char * epType() const { return "ImplementationError"; }
    virtual const char * epWhat() const { return what(); }
  };

  //assert which is always checked and throws logic errors:
  inline void assert_logic(bool b) {
    if (!b)
      throw LogicError("assertion failed");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                         //
// Macro's for easy and consistent throwing from within ExprParser code (including from    //
// derived classes (don't use THROW2 in tight CPU-critical code):                          //
//                                                                                         //
//   EXPRPARSER_THROW(ErrType,"some hardcoded message")                                    //
//   EXPRPARSER_THROW2(ErrType,"some "<<flexible<<" message")                              //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#define EXPRPARSER_THROW(ErrType, msg)   \
  {                                      \
    throw ::ExprParser::ErrType(msg);    \
  }

#define EXPRPARSER_THROW2(ErrType, msg)             \
  {                                                 \
    std::ostringstream err_oss;                     \
    err_oss << msg;                                 \
    EXPRPARSER_THROW(ErrType,err_oss.str().c_str()) \
  }


#endif
