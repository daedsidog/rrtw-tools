#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <string>
#include <vector>
#include <unordered_map>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

struct unit {
  bool mercenary;
  size_t lineno;
  std::string dictionary;
  std::vector<std::string> attributes;
  std::vector<std::string> owners;
  std::vector<std::string> soldiers;
  std::vector<std::string> officers;
};

struct texture {
  size_t lineno;
  std::string path;
};

struct battle_model {
  size_t lineno;
  std::string dictionary;
  std::unordered_map<std::string, texture> textures;
  std::unordered_map<std::string, texture> pbr_textures;
};

const std::string get_parent_dir(std::string_view path);

// Finds the mod root directory from given path.
const std::string get_mod_root_dir(std::string_view path);

std::vector<unit> parse_units(std::string_view export_descr_unit_file);

std::unordered_map<std::string, battle_model> parse_battle_models(std::string_view descr_model_battle_file);

// Reads an entire file into a string.
std::string freadall(std::string_view file);

enum color {
  DARK_RED,
  DARK_GREEN,
  DARK_YELLOW,
  DARK_BLUE,
  DARK_CYAN,
  DARK_GRAY,
  GRAY,
  MAGENTA,
  WHITE,
  BLACK,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  PINK,
  CYAN,
  ORANGE,
  MUSTARD,
};

// Describes the class which constructs colors based on given parameters.
class sgr {
  bool bold = false;
  bool underline = false;
  const char *text = nullptr;
  const char *fg_color = nullptr;
  const char *bg_color = nullptr;

public:
  sgr(const std::string_view str);
  sgr &fg(const color col);
  sgr &bg(const color col);
  sgr &bld(); // Bold text
  sgr &ul();  // Underline text
  std::string enc();
};

#endif
