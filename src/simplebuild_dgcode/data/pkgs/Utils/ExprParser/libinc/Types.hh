#ifndef ExprParser_Types_hh
#define ExprParser_Types_hh

#include <cstdint>
#include <string>

namespace ExprParser {

  typedef std::string str_type;
  typedef std::int64_t int_type;
  typedef double float_type;

  enum ExprType { ET_INT = 0, ET_FLOAT = 1, ET_STRING = 2 };

}

#endif
