#ifndef DCC_STRMAN_HPP
#define DCC_STRMAN_HPP

#include <string>
#include <vector>

namespace dcc {

  constexpr auto PRINTABLE_ASCII_START_CHAR = 32;
  constexpr auto PRINTABLE_ASCII_END_CHAR = 126;

  bool starts_with_digit(std::string_view);
  bool alphanumerical(std::string_view);
  bool alphanumerical(const char);
  bool atob(const char*);
  bool atob(const std::string&);
  bool is_whitespace(char);

  // Get rid of of all non-readable ASCII characters.
  std::string get_readable(std::string_view str, bool break_newline = false);
  std::string get_readable(std::string_view str, bool break_newline,
                           std::string& remainder);
  std::string upcase(std::string_view);

  // Returns a non-capitalized version of a given sentence. This is useful if we
  // want to integrate third-party error messages into our own error messaging
  // without mismatching the grammar.
  std::string uncap(std::string_view);

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

  std::vector<std::string> strtok(std::string_view str,
                                  std::string_view delims = ",\n\r\t ");

}; // namespace dcc

#endif
