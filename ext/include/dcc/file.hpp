#ifndef DCC_FILE_HPP
#define DCC_FILE_HPP

#include <dcc/debug.hpp>
#include <dcc/errno.hpp>
#include <dcc/logger.hpp>
#include <dcc/strman.hpp>
#include <filesystem>
#include <functional>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace dcc {

  namespace detail {

    namespace bom {

      const unsigned char utf8[] = {0xef, 0xbb, 0xbf};
      const unsigned char utf16be[] = {0xfe, 0xff};
      const unsigned char utf16le[] = {0xff, 0xfe};
      const unsigned char utf32be[] = {0, 0, 0xff, 0xfe};
      const unsigned char utf32le[] = {0xff, 0xfe, 0, 0};

    }; // namespace bom

    namespace regex {

      const std::string assignsyms = "=\\s\\t\\:\\n,";
      const std::string ignoresyms = "\\s\\t\\r\\n";
      const std::string set_leftcb = "^(.*?)\\{";
      const std::string set_rightcb = "^(.*?)\\}";
      const std::string entry_content = fmt::format("^(.*?)[{}]*$", ignoresyms);

    }; // namespace regex

  }; // namespace detail

  template <typename T, typename K = int> class parser {

    static_assert(std::is_class<T>::value,
                  "T must be a struct/class type with a default constructor.");

    struct set_entry {
      std::string prefix;
      std::string entries;
      std::vector<set_entry> nested_sets;
    };

  public:
    parser(std::string_view path) { _fpath = path; };

    int parse(T& t) {
      _container = ctype::singular;

      int res = _parse();
      if (res == 0)
        t = this->t;
      return res;
    };

    int parse(std::vector<T>& tvec) {
      _container = ctype::vector;
      if (partition.empty()) {
        dcc_logerr("No partition set for parsing {}.", sgr::file(_fpath));
        return -1;
      }

      int res = _parse();
      if (res == 0)
        tvec = this->_tvec;
      return res;
    };

    int parse(std::unordered_map<K, T>& tmap) {
      _container = ctype::map;
      if (partition.empty()) {
        dcc_logerr("No partition set for parsing {}.", sgr::file(_fpath));
        return -1;
      }

      int res = _parse();
      if (res == 0)
        tmap = this->_tmap;
      return res;
    };

  protected:
    std::string partition; // A partition determines at which entry/set should
                           // the parser create a new struct. This only applies
                           // for parsing to maps or vectors.
    std::string comment;   // How comments look in the file. Anything past this
                           // matched string in the same line is discarded.
    std::unordered_map<std::string, std::function<void()>> entries;
    std::unordered_map<std::string, std::function<void()>> sets;
    std::function<K()> key = [this]() {
      _key_undefined = true;
      return K();
    };
    T t;

    const std::string& entry() { return _entry; }
    const set_entry& set() { return _set; };

    size_t lineno() { return _hit_lineno; }

  private:
    enum class ctype { singular, vector, map };
    enum class encoding { undefined, utf8, utf16, utf32 };
    enum byte_pos { bom, byte1, byte2, byte3, byte4 };

    bool _t_empty = true;
    bool _parsing_set = false;
    bool _key_undefined = false;
    bool _partition_hit = false;
    size_t _cb_idx = 0;
    size_t _lineno = 0;
    size_t _hit_lineno = 0;
    size_t _entry_hits = 0;
    size_t _set_hits = 0;
    size_t _cb_lineno = 0;
    size_t _set_start_lineno = 0;
    ctype _container = ctype::singular;
    set_entry _set;
    std::string _entry;
    std::string _fpath;
    std::string _set_key;
    std::string _prefix;
    std::vector<set_entry*> _setnest;
    std::vector<T> _tvec;
    std::unordered_map<std::string, std::function<void()>> _callbacks;
    std::unordered_map<K, T> _tmap;
    std::unordered_map<std::string, std::string> _entries_regex;
    std::unordered_map<std::string, std::string> _sets_regex;

    int _process_line(std::string& line) {

      // An entire line has been parsed.
      // Our job currently is to process thin line. We first start by
      // determining whether or not we are currently processing a set, or a
      // regular entry. If we are processing a regular entry, our sole objective
      // is to parse the content from that singular line. However, if we are
      // parsing a set, we are not guaranteed that the entire set is written out
      // on that one line, or that the line does not contain multiple sets.
      // Therefore, we would have to process the line repeatedly, until all
      // cases are handled.
      std::smatch m;
      std::regex r;
      while (not line.empty()) {

        // We are currently not handling a set from previous lines.
        if (not _parsing_set) {

          // This is the case where all the information in regards to the entry
          // is present in a single line. In other words, the current line is
          // not part of a set. The parsing task here is relatively simple.
          for (const auto& [entry_key, entry_callback] : entries) {
            r = std::regex(_entries_regex[entry_key]);
            if (std::regex_search(line, m, r)) {
              if (entry_key == partition) {
                if (not _partition_hit)
                  _partition_hit = true;
                else {
                  switch (_container) {
                  case ctype::vector:
                    _tvec.push_back(t);
                    t = T();
                    break;
                  case ctype::map: {
                    K k = key();
                    if (_key_undefined) {
                      dcc_logerr("No key lambda was defined to map {}.",
                                 sgr::file(_fpath));
                      return -1;
                    }
                    _tmap[k] = t;
                    t = T();
                  } break;
                  default:
                    break;
                  }
                }
              }

              std::string content =
                line.substr(m.length(0), line.length() - m.length(0));
              r = std::regex(detail::regex::entry_content);
              std::regex_search(content, m, r);
              _entry = m[1];
              _entry_hits += 1;
              _hit_lineno = _lineno;
              if (not _entry.empty())
                entry_callback();
              return 0;
            }
          }

          // The current line has not been matched as a regular entry, and
          // therefore we should now check whether or not it is a set entry.
          bool matched_set = false;
          for (const auto& [set_key, set_callback] : sets) {
            r = std::regex(_sets_regex[set_key]);
            if (std::regex_search(line, m, r)) {

              // We matched a set. The set may or may not have the curly braces
              // on this line, or it may have them in the next lines. Until we
              // match a left curly brace, we will add all the content until
              // then to the entries of the set.
              if (set_key == partition) {
                if (not _partition_hit)
                  _partition_hit = true;
                else {
                  switch (_container) {
                  case ctype::vector:
                    _tvec.push_back(t);
                    t = T();
                    break;
                  case ctype::map: {
                    K k = key();
                    if (_key_undefined) {
                      dcc_logerr("No key lambda was defined to map {}.",
                                 sgr::file(_fpath));
                      return -1;
                    }
                    _tmap[k] = t;
                    t = T();
                  } break;
                  default:
                    break;
                  }
                }
              }
              _parsing_set = true;
              _set_start_lineno = _lineno;
              _hit_lineno = _lineno;
              _set_key = set_key;
              line = line.substr(m.length(0), line.length() - m.length(0));
              matched_set = true;
              break;
            }
          }
          if (matched_set)
            continue;
        }

        // We have matched a set in the previous lines that we did not finish
        // parsing.
        else {

          // Does this line have a left curly brace?
          r = std::regex(detail::regex::set_leftcb);
          if (std::regex_search(line, m, r)) {

            // Is it the root set?
            if (_setnest.empty()) {
              _set = set_entry();
              _set.prefix = _prefix;
              _setnest.clear();
              _setnest.push_back(&_set);

              // Is it a nested set?
            } else {
              _setnest.back()->nested_sets.push_back(set_entry());
              _setnest.push_back(&(_setnest.back()->nested_sets.back()));
            }
            _cb_idx += 1;
            _setnest.back()->prefix += m[1];
            line = line.substr(m.length(0), line.length() - m.length(0));
            continue;
          }

          // Does this line have a right curly brace?
          r = std::regex(detail::regex::set_rightcb);
          if (std::regex_search(line, m, r)) {
            if (_cb_idx == 0) {
              dcc_logerr("Unexpected closing brace in line {}.", _lineno);
              return -1;
            }
            _cb_idx -= 1;
            _setnest.back()->entries += m[1];
            _setnest.pop_back();
            line = line.substr(m.length(0), line.length() - m.length(0));

            // Have we parsed the entire matched set?
            if (_cb_idx == 0) {
              _parsing_set = false;
              auto it = sets.find(_set_key);
              dcc_assert(it != sets.end());
              it->second();
              _set_hits += 1;

              // Although we finished parsing the set, there may still be
              // something more on this line, such as more sets.
              continue;
            }
          }

          // If we reached this point, this mean that no braces were found at
          // this line. We should append whatever we find to the contents of the
          // current set, or its prefix if no set has yet been found.
          if (not _setnest.empty())
            _setnest.back()->entries += line;
          else
            _prefix += line;
          return 0;
        }

        // If we reached this point, it means that this line has nothing of
        // interest to us.
        break;
      }
      return 0;
    }

    int _parse() {
      dcc_logdbg("Parsing {}...", sgr::file(_fpath));
      if (not std::filesystem::exists(_fpath)) {
        dcc_logerr("Could not find {} for parsing.", sgr::file(_fpath));
        return -1;
      }

      FILE* f = fopen(_fpath.data(), "r");
      if (not f) {
        dcc_logerr("Could not open {} for parsing: {}.", sgr::file(_fpath),
                   errmsg());
        return -1;
      }
      _parsing_set = false;
      _cb_idx = 0;
      _entry_hits = 0;
      _set_hits = 0;
      t = T();
      _entries_regex.clear();
      _sets_regex.clear();
      _prefix.clear();

      bool hit_comment = false;
      char buf[BUFSIZ];
      size_t bytes_read;
      size_t read_offset = 0;
      size_t commlen = comment.length();
      byte_pos bpos = bom;
      encoding enc = encoding::undefined;
      std::string line;

      // Generate all the necessary RegEx expressions.
      for (const auto& [entry_key, _] : entries)
        _entries_regex.emplace(
          entry_key,
          fmt::format("^[{}]*{}[{}]*[{}][{}]*", detail::regex::ignoresyms,
                      entry_key, detail::regex::ignoresyms,
                      detail::regex::assignsyms, detail::regex::ignoresyms,
                      detail::regex::ignoresyms));
      for (const auto& [set_key, _] : sets)
        _sets_regex.emplace(
          set_key,
          fmt::format("^[{}]*{}[{}]*[{}][{}]*", detail::regex::ignoresyms,
                      set_key, detail::regex::ignoresyms,
                      detail::regex::assignsyms, detail::regex::ignoresyms));

      // Read file to buffer, process buffer into lines, then process the lines.
      do {
        bytes_read = fread(buf, 1, BUFSIZ, f);
        if (bytes_read == 0)
          break;
        if (enc == encoding::undefined) {
          if (bytes_read >= sizeof(detail::bom::utf8) and
              memcmp(buf, detail::bom::utf8, sizeof(detail::bom::utf8)) == 0) {
            read_offset = sizeof(detail::bom::utf8);
            enc = encoding::utf8;
          } else if (bytes_read >= sizeof(detail::bom::utf16be) and
                     memcmp(buf, detail::bom::utf16be,
                            sizeof(detail::bom::utf16be)) == 0) {
            read_offset = sizeof(detail::bom::utf16be);
            enc = encoding::utf16;
          } else if (bytes_read >= sizeof(detail::bom::utf16le) and
                     memcmp(buf, detail::bom::utf16le,
                            sizeof(detail::bom::utf16le)) == 0) {
            read_offset = sizeof(detail::bom::utf16le);
            enc = encoding::utf16;
          } else if (bytes_read >= sizeof(detail::bom::utf32be) and
                     memcmp(buf, detail::bom::utf32be,
                            sizeof(detail::bom::utf32be)) == 0) {
            read_offset = sizeof(detail::bom::utf32be);
            enc = encoding::utf32;
          } else if (bytes_read >= sizeof(detail::bom::utf32le) and
                     memcmp(buf, detail::bom::utf32le,
                            sizeof(detail::bom::utf32le)) == 0) {
            read_offset = sizeof(detail::bom::utf32le);
            enc = encoding::utf32;
          } else
            enc = encoding::utf8;
        } else if (read_offset != 0)
          read_offset = 0;
        for (size_t i = read_offset; i < bytes_read; ++i) {
          switch (bpos) {
          case bom:
          case byte4:
            bpos = byte1;
            break;
          case byte1:
            bpos = byte2;
            break;
          case byte2:
            bpos = byte3;
            break;
          case byte3:
            bpos = byte4;
            break;
          }
          switch (enc) {
          case encoding::utf16:
            if (bpos == byte1 or bpos == byte3)
              continue;
            break;
          case encoding::utf32:
            if (bpos != byte4)
              continue;
            break;
          default:
            break;
          }

          if (hit_comment and buf[i] != '\n')
            continue;
          line += buf[i];
          if (not hit_comment and not comment.empty()) {

            // Check if we did not add a comment. If we did, we have to remove
            // it and process the line.
            size_t linelen = line.length();
            if (linelen >= commlen) {
              hit_comment = true;
              for (size_t j = 0; j < commlen; ++j) {
                if (line[linelen - commlen + j] != comment[j]) {
                  hit_comment = false;
                  break;
                }
              }
            }
            if (hit_comment)
              line = line.substr(0, linelen - commlen);
          }
          if (buf[i] == '\n') {
            _lineno += 1;
            if (_process_line(line) != 0)
              return -1;
            line.clear();
            hit_comment = false;
          }
        }
      } while (bytes_read == BUFSIZ);
      if (not line.empty())
        if (_process_line(line) != 0)
          return -1;
      if (_cb_idx != 0) {
        dcc_logerr("Unterminated set {} in line {}.", sgr::semiunique(_set_key),
                   _set_start_lineno);
        return -1;
      } else if (_parsing_set) {
        dcc_logerr("Malformed set {} in line {}.", sgr::semiunique(_set_key),
                   _set_start_lineno);
        return -1;
      }
      dcc_logdbg("Parsed entries: {}", _entry_hits);
      dcc_logdbg("Parsed sets:    {}", _set_hits);
      switch (_container) {
      case ctype::vector:
        _tvec.push_back(t);
        dcc_logdbg("Parsed structs: {}", _tvec.size());
        break;
      case ctype::map: {
        K k = key();
        if (_key_undefined) {
          dcc_logerr("No key lambda was defined to map {}.", sgr::file(_fpath));
          return -1;
        }
        _tmap[k] = t;
        dcc_logdbg("Parsed structs: {}", _tmap.size());
      } break;
      default:
        break;
      }
      return 0;
    };
  };

  // Create a directory tree using a given path.
  int create_dirtree(const char* dirtree);

  // Reads an entire file into a given string. Truncates any null bytes.
  // Returns 0 on success, -1 on error;
  int freadall(const char* path, std::string& outstr);

  // Compare two files.
  // Return 0 if they are equal, -1 if error (sets errno) and 1 if the
  // files are different.
  //
  // If given diffpos, will set it to be the position of the first byte
  // where the bytes are different.
  int fcmp(const char* src, const char* dst);
  int fcmp(const char* src, const char* dst, size_t& diffpos);
  int fcmp(const std::string& src, const std::string& dst);
  int fcmp(const std::string& src, const std::string& dst, size_t& diffpos);

  // If the directory structure described by 'dir' does not exist, this
  // function will create it, and then attempt to open the file 'logname'
  // from inside that directory.
  FILE* dirgen_fopen(const char* logname, const char* dir, const char* flags);

}; // namespace dcc

#endif
