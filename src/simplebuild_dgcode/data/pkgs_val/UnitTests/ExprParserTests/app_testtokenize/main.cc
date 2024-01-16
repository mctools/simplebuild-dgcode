#include "ExprParser/Tokenizer.hh"
#include "ExprParser/Exception.hh"
#include <cstdio>
#include <stdexcept>

int main(int,char**) {
  namespace EP=ExprParser;
  EP::TokenList v;
  std::vector<std::string> teststrings
    = {
    "",
    "   'dsfsdf sdf  '     'sdfsdf' ",
    " 'dsfsdf sdf  ' 'sdfsdf' ' ' ",
    "   'dsfsdf sdf  '     'sdfs\\'df'  ",
    "'\\''   'dsf\"sdf sdf  '  \"'sdf'\"   'sdfs\\'df' '\\''",
    "'A string' .34e+12 (17.0)",
    "'A ()string' .34e+12 (17.0)",
    "'A ()string' .34e+12 2pname2_b(17.0)",
    "'A string' || -.34e+12 +(17.0)> (5.0)",
    "'A string' || -.34e+12 +(17.0)> (5.0-1)",
    "5.0 meV",
    "5.0meV",
    "5.0 eV",
    "5.0eV",
    "5.0 E",
    "5.0E",
    "5.0 - 1",
    "5.0-1",
    "5.0E-1",
    "5.0E- 1",
    "5.0E -1",
    "5.0E- 1",
    "5.0E-",
    "-5.0E-",
    "test && 0x456F",
    "0x40F5a6 0744 744 0b101",
    "9223372036854775807",//highest 64-bit long value
    "0b0111111111111111111111111111111111111111111111111111111111111111",//same in binary
    "0B111111111111111111111111111111111111111111111111111111111111111",//same again
    "0x7FFFFFFFFFFFFFFF",//same in hex
    "0X7ffffffffffffffe",//one less
    "0X8000000000000000",//one more (giving lowest 64-bit long value)
    "0xFFFFFFFFFFFFFFFF",//minus one
    "0b1111111111111111111111111111111111111111111111111111111111111111",//minus one again
    "0x000000000000",//zero
    "0b000000000000",//zero
    "0x0000000000000000000000011",//17 (even though more than 64bits specified)
    "1+-2",
    "1+----2",
    "1+---+2",
    "1&&!0",
    "1!&&!0",
    "1!=0",
    "2e",
    "2e-1",
    "2e - 5",
    "bla.lala",
    "bla . lala",
    "bla .lala",
    "bla. lala",
    "bla.la.la",
    "bla..lala",
    ".bla",
    "bla .2",
    "1.0e9",
    "1.0E+9",
    "1.0e-9",
    "1e9",
    "1E+9",
    "1e-9",
    "0b9",//not clear what would be the desired tokenisation here...
    "0xG",//not clear what would be the desired tokenisation here...
    "09",
    "0b151"

  };

  for (size_t i = 0; i < teststrings.size(); ++i) {
    printf("Tokenizing >>%s<<:\n",teststrings.at(i).c_str());
    try {
      EP::createTokens(teststrings.at(i),v);
      for (auto it = v.begin(); it!=v.end(); ++it)
        printf("    => token[%i] >>%s<<\n",(*it)->rawtype(),(*it)->content().c_str());
    } catch (EP::InputError& e) {
      printf("Tokenization gives exception: %s : %s\n",e.epType(),e.epWhat());
    }
  }
  return 0;

}

