#include "ExprParser/Tokenizer.hh"
#include "ExprParser/Exception.hh"
#include <sstream>
#include <cstdlib>//strtol
#include <iomanip>//strtol
#include <stdexcept>

#ifdef INSTR
#  undef INSTR
#endif
#define INSTR(achar,astring) (astring.find(achar)!=str_type::npos)

namespace ExprParser {

  //NB: Because of scientific notation, we disallow identifiers named "e" or "E"???
  //todo: allow special number inf (nah, common constant)?
  //NB: for the purposes of expression parsing, underscores are treated like any
  //other letter a-z.
  bool is_alpha(char c) { return (c>='a'&&c<='z') || (c>='A'&&c<='Z') || c=='_'; }
  bool is_num(char c) { return c>='0' && c<='9'; }
  bool is_octnum(char c) { return c>='0' && c<='7'; }
  bool is_hexnum(char c) { return (c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F'); }
  bool is_binnum(char c) { return c=='0' || c=='1'; }
  bool is_num(char c, int base) { assert_logic(base==16||base<=10); return base == 16 ? is_hexnum(c) : (c>='0' && c<=(('0'-1)+base)); }
  bool is_alphanum(char c) { return is_alpha(c) || is_num(c) || c=='.'; }
  template<class TVal>
  bool str2val(const str_type& s, TVal& result, int base = 10)
  {
    assert_logic(base==10||base==8||base==16);//setbase does not support other bases
    std::stringstream ss(s);
    ss >> std::setbase(base) >> result;
    return !ss.fail() && ss.eof();
  }
  template<class TVal>
  bool str2val(const str_type& s, int base = 10) { TVal res; return str2val(s,res,base); }

  void createTokens(const str_type& input, TokenList& l) {
    l.clear();//or just append?
    const str_type valnum_begin = "0123456789.";
    const str_type valnum_all = "0123456789.Ee-+";
    const str_type func_commas = ",;";
    const str_type symb_true = "%:^&|!*~/+-<>=?";
    const str_type single_ops = "+-!~";//always as single char symbol, unless followed by '='
    const str_type valstr_delims = "'\"";
    const str_type whitespace=" \t\n\r";
    const char * c = input.c_str();
    const char * cE = c + input.size();
    str_type tmp;
    for (;c!=cE;++c) {
      assert_logic(c<cE);
      //skip whitespace:
      while ( c!=cE && INSTR(*c,whitespace) )
        ++c;
      if (c==cE)
        break;
      assert_logic(c<cE);
      if ( INSTR(*c,func_commas) ) {
        l.push_back(Token::createComma());
        continue;
      }
      if (*c=='(') {
        l.push_back(Token::createLeftParen());
        continue;
      }
      if (*c==')') {
        l.push_back(Token::createRightParen());
        continue;
      }
      //test if a litteral string starts here and consume it if it does:
      if ( INSTR(*c,valstr_delims) ) {
        assert_logic(c<cE);
        char delim = *c;
        const char * c2 = c+1;
        assert_logic(c2&&c2<cE);
        tmp.clear();
        while (c2 != cE && *c2 != delim)
          {
            if (*c2 == '\\' && c2+1!=cE && *(c2+1)==delim) {
              //escaped delim, flush what we got so far:
              if (c2>c+1) {
                tmp += str_type(c+1,c2-c-1);
                c=c2;
              }
              //and skip past the back-slash and delimiter:
              ++c2;
            }
            ++c2;
          }
        if (c2==cE)
          EXPRPARSER_THROW2(ParseError, "non-terminated string constant : " << c)
        assert_logic(c2<cE);
        //flush:
        if (c2>c+1) {
          assert_logic(c2-c-1>0&&c+1<cE);
          str_type test(c+1,c2-c-1);
          tmp += str_type(c+1,c2-c-1);
        }
        l.push_back(Token::createLitteralValueString(tmp));
        tmp.clear();
        c=c2;
        assert_logic(c<cE);
        continue;
      }
      if ( is_num(*c) || *c=='.' ) {
        //number start
        bool saw_period = (*c=='.');
        bool saw_eE = false;
        int base = 10;
        int prefix_len = 0;
        //detect octal numbers (preceded with '0'), hexadecimal numbers
        //(preceded with '0x') and numbers in binary representations (preceded
        //with '0b'):
        if (*c == '0' && c+1 != cE ) {
          if ( *(c+1)=='8' || *(c+1)=='9' )
            EXPRPARSER_THROW2(ParseError, "Digit "<<*(c+1)<<" is out of range in octal numbers")
          if ( is_octnum( *(c+1) ) ) {
            base = 8;
            prefix_len = 1;
          } else if (c+2 != cE) {
            if ( (*(c+1)=='x'||*(c+1)=='X') && is_hexnum(*(c+2)) ) {
              base = 16;
              prefix_len = 2;
            } else if ( (*(c+1)=='b'||*(c+1)=='B') && is_binnum(*(c+2)) ) {
              base = 2;
              prefix_len = 2;
            }
          }
        }
        const char * c2 = c + (prefix_len==0?1:prefix_len);
        while ( c2 != cE ) {
          if (is_num(*c2)&&!is_num(*c2,base))
            EXPRPARSER_THROW2(ParseError, "Digit "<<*c2<<" is out of range in base-"<<base<<" numbers")
          if (is_num(*c2,base)) {
            ++c2;
            continue;
          }
          if (base!=10)
            break;//only support floats in base 10
          if (*c2=='.' && !saw_eE && !saw_period) {
            saw_period=true;
            ++c2;
            continue;
          }
          if ( (*c2=='e' || *c2=='E') && !saw_eE) {
            //consume if followed by digit or +/- and then a digit
            if (c2+1!=cE) {
              if ( c2+2!=cE && (*(c2+1)=='-'||*(c2+1)=='+') && is_num(*(c2+2)) ) {
                //+ or - right after e/E and then a digit
                saw_eE=true;
                c2 += 3;
                continue;
              } else if (is_num(*(c2+1))) {
                //a digit right after e/E
                saw_eE=true;
                c2 += 2;
                continue;
              }
            }
          }
          break;
        }
        assert_logic(c+prefix_len < c2);
        c += prefix_len;
        tmp.assign(c,c2-c);
        if (saw_period||saw_eE) {
          //floating point
          assert_logic(base==10);
          if (!str2val<float_type>(tmp))
            EXPRPARSER_THROW2(ParseError, "malformed floating point constant in input : " << tmp)
          l.push_back(Token::createLitteralValueFloat(tmp));
        } else {
          const char * errmsg_badint = "malformed integer constant in input : ";
          if (base==2||base==8) {
            //avoid some non-intuitive parsing results by erroring if binary or
            //octal numbers are followed by out-of-base digits:
            if (c2!=cE && *c2<='9' && *c2 >= '0'+base)
              EXPRPARSER_THROW2(ParseError, errmsg_badint << tmp)
          }
          //decode as unsigned, before storing as signed (to allow base=2,8,16
          //bit patterns to include those with the sign bit set).
          std::uint64_t val(0);
          if (base==2) {
            //hand-crafted decoding of binary representation (not that hard).
            if (tmp.size()>64||tmp.size()<1)
              EXPRPARSER_THROW2(ParseError, errmsg_badint << tmp)
            std::uint64_t pow = 1;
            for (auto it = tmp.rbegin(); it!=tmp.rend(); ++it) {
              if (*it == '1')
                val += pow;
              else if (*it != '0')
                EXPRPARSER_THROW2(ParseError, errmsg_badint << tmp)
              pow *= 2;
            }
          } else {
            if (!str2val<std::uint64_t>(tmp,val,base))
              EXPRPARSER_THROW2(ParseError, errmsg_badint << tmp)
          }
          if (base==10&&val>9223372036854775807)
            EXPRPARSER_THROW2(ParseError,"integer constant out of range in input : "<<tmp);
          int_type valu = (int_type)val;
          if (base!=10) {
            std::ostringstream oss;
            oss << valu;
            tmp = oss.str();
          }
          l.push_back(Token::createLitteralValueInteger(tmp));
        }
        c=c2-1;
        continue;
      }
      if ( is_alpha(*c) ) {
        const char * c2 = c+1;
        while ( c2!=cE && (is_alphanum(*c2) || *c2=='.') ) {
          //'.' only allowed if not ending token and not immediately following another dot.
          if (*c2=='.'&&*(c2-1)=='.')
            EXPRPARSER_THROW(ParseError,"identifiers are not allowed to contain consecutive dots");
          ++c2;
        }
        if (*(c2-1)=='.')
          EXPRPARSER_THROW(ParseError,"identifiers are not allowed to end with a dot");
        l.push_back(Token::createIdentifierNamed(tmp.assign(c,c2-c)));
        c=c2-1;
        continue;
      }
      if ( INSTR(*c,symb_true) ) {
        const char * c2 = c+1;
        while ( c2!=cE && INSTR(*c2,symb_true) ) {
          ++c2;
        }
        while (c2-c > 1) {
          //multi-character, figure out how to split is a bit tricky, e.g.
          //                        5+-2 (should give 3, not some "+-" operator)
          //                        5*-2 (should give -10, not some "*-" operator)
          //                        &&!bla (should give &&(!bla), not some "&&!" operator)
          //                        5+--+---2 should give 3
          //                        5+--+---2 should give 3, as should 5+-----2,
          //                        but 5+--2 should give 7
          //We implement the case where such unary operators are never used in
          //multi-character operators, expect if followed by '=':
          const char * c3 = c;
          while (c3!=c2&&INSTR(*c3,single_ops)) {
            ++c3;//consume exactly one single-op char
            if (c3!=cE && *c3=='=') {
              ++c3;//special case, make sure that +=, -=, ~=, != becomes one symbol anyway
            }
            l.push_back(Token::createIdentifierSymbolic(tmp.assign(c,c3-c)));
            c=c3;
          }
          while (c3!=c2 && !INSTR(*c3,single_ops)) {
            ++c3;//consume any number of non-single-ops chars...
          }
          if (c3>c) {
            l.push_back(Token::createIdentifierSymbolic(tmp.assign(c,c3-c)));
            c=c3;
          }
        }
        if (c2-c==1)
          l.push_back(Token::createIdentifierSymbolic(tmp.assign(c,c2-c)));
        c=c2-1;
        continue;
      }

      EXPRPARSER_THROW2(ParseError,"unknown characters in input : " << *c);
    }

  }
}

