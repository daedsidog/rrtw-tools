#ifndef RRT_COMMON_HPP
#define RRT_COMMON_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

struct strat_model_entry {
  size_t lineno;
  std::string strat_card;
  std::unordered_set<std::string> owners;
};

struct strat_model {
  size_t lineno;
  std::string path;
  std::string nv_path;
  std::unordered_map<std::string, texture> textures;
  std::unordered_map<std::string, texture> pbr_textures;
};

struct banner {
  std::unordered_set<std::string> models;
  std::unordered_set<std::string> outlines;
};

const std::string get_parent_dir(std::string_view path);

// Finds the mod root directory from given path.
const std::string get_mod_root_dir(std::string_view path);

std::vector<unit> parse_units(std::string_view export_descr_unit_fname);

std::unordered_map<std::string, battle_model>
parse_battle_models(std::string_view descr_model_battle_fname);

std::unordered_map<std::string, strat_model_entry>
parse_strat_model_entries(std::string_view descr_model_battle_fname);

std::unordered_map<std::string, strat_model>
parse_strat_models(std::string_view descr_model_strat_fname);

std::unordered_map<std::string, banner>
parse_banners(std::string_view descr_banners_fname);

#endif
