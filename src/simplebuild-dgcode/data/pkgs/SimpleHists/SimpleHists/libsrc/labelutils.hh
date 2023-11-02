

namespace SimpleHists {

  bool is_alpha(char c)
  {
    return (c>='A'&&c<='z'&&(c>='a'||c<='Z'));
  }

  bool is_digit(char c)
  {
    return c>='0' && c<= '9';
  }
  bool is_alpha_or_underscore(char c)
  {
    return c=='_' || is_alpha(c);
  }

  bool is_valid_python_identifier(const std::string& s, bool can_be_private = true)
  {
    if (s.empty())
      return false;
    const char * c = &(s[0]);
    if (can_be_private) {
      if (!is_alpha_or_underscore(*c))
        return false;
    } else {
      if (!is_alpha(*c))
        return false;
    }
    const char * cE = c + s.size();
    for (++c;c!=cE;++c)
      if (!(is_alpha_or_underscore(*c)||is_digit(*c)))
        return false;
    return true;
  }

  void convert_spaces_to_underscores(std::string& s) {
    char * c = &(s[0]);
    char * cE = c + s.size();
    for (;c!=cE;++c) {
      if (*c==' ')
        *c = '_';
    }
  }

  void convert_to_lowercase(std::string& s) {
    char * c = &(s[0]);
    char * cE = c + s.size();
    for (;c!=cE;++c) {
      if (*c>='A' && *c<='Z')
        *c = *c + ('a'-'A');
    }
  }


}
