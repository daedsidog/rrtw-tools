#ifndef DCC_STRMAN_HPP
#define DCC_STRMAN_HPP

#include <string>

namespace dcc {

  constexpr auto COMMENT_CHAR = ';';
  constexpr auto NEWLINE_CHAR = '\n';
  constexpr auto DQUOTE_CHAR = '"';
  constexpr auto RCBRACE_CHAR = '{';
  constexpr auto LCBRACE_CHAR = '}';
  constexpr auto WHITESPACE_CHAR = ' ';
  constexpr auto UNDERSCORE_CHAR = '_';
  constexpr auto CR_CHAR = '\r';
  constexpr auto PRINTABLE_ASCII_START_CHAR = 32;
  constexpr auto PRINTABLE_ASCII_END_CHAR = 126;
  constexpr auto UPPERCASE_ALPHABET_START = 'A';
  constexpr auto UPPERCASE_ALPHABET_END = 'Z';
  constexpr auto LOWERCASE_ALPHABET_START = 'a';
  constexpr auto LOWERCASE_ALPHABET_END = 'z';
  constexpr auto DIGIT_START = '0';
  constexpr auto DIGIT_END = '9';

  void strip_whitespaces(std::string& str);
  void strto_ucstr(std::string& str);
  std::string strip_whitespaces(const std::string& str);
  std::string strto_ucstr(const std::string& str);

  bool starts_with_digit(const std::string& str);
  bool alphanumerical(const std::string& str);
  bool alphanumerical(const char c);
  bool atob(const char* str);
  bool atob(const std::string& str);

  // Get rid of of all non-readable ASCII characters.
  std::string purne_string(std::string_view str, bool break_newline = false);
  std::string purne_string(std::string_view str, bool break_newline,
                           std::string& remainder);

  // Returns a non-capitalized version of a given sentence. This is useful if we
  // want to integrate third-party error messages into our own error messaging
  // without mismatching the grammar.
  std::string uncapsent(std::string_view sent);

  // The purpose of this function is to provide a general method of formatting
  // error response strings to be displayed properly within custom error
  // messages.
  //
  // For example: OpenGL returns shader errors in this format:
  //
  //     "ERROR: Something went wrong\n"
  //
  // Printing this string after your own custom message, for example
  //
  //     "Something went wrong, reason: <OpenGL error>\n"
  //
  // would not really format well. So here comes this function to the rescue.
  std::string reasonify(std::string_view sent);

  float digit_ratio(const std::string& str);

  template <class T, typename D>
  std::string join_strings(T strings, D delimiter) {
    bool first = true;
    std::string joined_str;
    for (const auto& str : strings) {
      if (not first)
        joined_str += delimiter + str;
      else {
        first = false;
        joined_str = str;
      }
    }
    return joined_str;
  }

}; // namespace dcc

#endif
