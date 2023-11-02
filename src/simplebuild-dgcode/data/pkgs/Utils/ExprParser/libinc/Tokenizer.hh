#ifndef ExprParser_Tokenizer_hh
#define ExprParser_Tokenizer_hh

#include "ExprParser/Types.hh"
#include <memory>
#include <vector>

namespace ExprParser {

  //Tokens, to hold initial parsing results, before contextual interpretation:

  class Token;
  typedef std::shared_ptr<Token> TokenPtr;
  typedef std::vector<TokenPtr> TokenList;

  class Token {
  public:
    //Build:
    static TokenPtr createLeftParen() { return TokenPtr(new Token(0,"(")); }
    static TokenPtr createRightParen() { return TokenPtr(new Token(1,")")); }
    static TokenPtr createLitteralValueString(const str_type& s) { return TokenPtr(new Token(2,s)); }//todo: accept with move?
    static TokenPtr createLitteralValueInteger(const str_type& s) { return TokenPtr(new Token(3,s)); }
    static TokenPtr createLitteralValueFloat(const str_type& s) { return TokenPtr(new Token(4,s)); }
    static TokenPtr createIdentifierSymbolic(const str_type& s) { return TokenPtr(new Token(5,s)); }
    static TokenPtr createIdentifierNamed(const str_type& s) { return TokenPtr(new Token(6,s)); }
    static TokenPtr createComma() { return TokenPtr(new Token(7,",")); }
    //Query:
    bool isParen() const { return m_type == 0 || m_type == 1; }
    bool isLeftParen() const { return m_type == 0; }
    bool isRightParen() const { return m_type == 1; }
    bool isComma() const { return m_type == 7; }
    bool isLitteralValue() const { return m_type >= 2 && m_type <= 4; }
    bool isLitteralValueString() const { return m_type == 2; }
    bool isLitteralValueInteger() const { return m_type == 3; }
    bool isLitteralValueFloat() const { return m_type == 4; }
    bool isIdentifier() const { return m_type == 5 || m_type == 6; }
    bool isIdentifierNamed() const { return m_type == 6; }
    const str_type& content() const { return m_cont; }
    int rawtype() const { return m_type; }
  private:
    int m_type;
    str_type m_cont;
    Token(int tt, const str_type& ss) : m_type(tt), m_cont(ss) {}
  };

  //Turn input string into a list of tokens:
  void createTokens(const str_type& input, TokenList& output);
}

#endif
