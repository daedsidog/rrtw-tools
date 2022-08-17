#ifndef RRT_COMMON_HPP
#define RRT_COMMON_HPP

#include <string>
#include <vector>
#include <unordered_map>

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

#endif
