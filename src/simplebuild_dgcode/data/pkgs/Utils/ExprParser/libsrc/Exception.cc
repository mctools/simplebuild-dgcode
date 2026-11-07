#include "ExprParser/Exception.hh"

//Out-of-line destructors, so the type information of the exception classes is
//only present in this library (see Exception.hh):
ExprParser::Exception::~Exception() throw() {}
ExprParser::InputError::~InputError() throw() {}
ExprParser::ParseError::~ParseError() throw() {}
ExprParser::DomainError::~DomainError() throw() {}
ExprParser::LogicError::~LogicError() throw() {}
