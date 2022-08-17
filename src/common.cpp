#include <cassert>
#include <cstdio>
#include <dcc/file.hpp>
#include <dcc/logger.hpp>
#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_set>

#include "common.hpp"
#include "dcc/fmt/color.h"

using namespace std;
using namespace dcc;

vector<unit> parse_units(string_view edu_path) {
  ifstream f(edu_path.data());
  if (not f.is_open()) {
    logerr("Could not open {}.", styled(edu_path, fmt::emphasis::underline));
    exit(-1);
  }

  bool first_unit = true;
  bool parsing_soldiers = false;
  size_t lineno = 0;
  size_t soldiers_brace_num = 0;
  string line;
  vector<unit> units;
  unit current_unit;
  unordered_set<string> added_officers;
  while (getline(f, line)) {
    ++lineno;

    smatch m;
    if (parsing_soldiers) {
      if (regex_search(line, m, regex("\\{")))
        soldiers_brace_num += 1;
      else if (regex_search(line, m, regex("\\}"))) {
        soldiers_brace_num -= 1;
        if (soldiers_brace_num == 0)
          parsing_soldiers = false;
      } else if (soldiers_brace_num == 2 and
                 regex_search(line, m, regex("([\\w\\d_]+)")))
        current_unit.soldiers.push_back(m[1]);
    } else if (regex_search(line, m, regex("dictionary\\s+([\\w\\d_]+)"))) {
      if (not first_unit) {
        assert(not current_unit.dictionary.empty());
        assert(current_unit.lineno != 0);
        units.push_back(current_unit);
        added_officers.clear();
      } else
        first_unit = false;
      current_unit = unit{.mercenary = false,
                          .lineno = lineno,
                          .dictionary = m[1],
                          .attributes = {},
                          .owners = {},
                          .soldiers = {},
                          .officers = {}};
    } else if (regex_search(line, m, regex("attributes\\s+(.+)"))) {
      string res = m[1];
      string token;
      istringstream is(res);
      while (getline(is, token, ',')) {
        token = regex_replace(token, regex("[\\r\\n\\s]"), "");
        current_unit.attributes.push_back(token);
        if (token == "mercenary_unit")
          current_unit.mercenary = true;
      }
    } else if (regex_search(line, m, regex("ownership\\s+(.+)"))) {
      string res = m[1];
      string token;
      istringstream is(res);
      while (getline(is, token, ',')) {
        token = regex_replace(token, regex("[\\r\\n\\s]"), "");
        current_unit.owners.push_back(token);
      }
    } else if (regex_search(line, m, regex("soldiers")))
      parsing_soldiers = true;
    else if (regex_search(line, m, regex("soldier\\s+([\\d\\w_]+)")))
      current_unit.soldiers.push_back(m[1]);
    else if (regex_search(line, m, regex("officer\\s+([\\d\\w_]+)"))) {
      string officer = m[1];
      if (not added_officers.contains(officer)) {
        current_unit.officers.push_back(m[1]);
        added_officers.insert(officer);
      }
    }
  }

  // Don't forget to push the last unit.
  if (not current_unit.dictionary.empty()) {
    assert(current_unit.lineno != 0);
    units.push_back(current_unit);
  }
  if (units.empty()) {
    logerr("No units were parsed.");
    exit(-1);
  }
  return units;
}

unordered_map<string, battle_model> parse_battle_models(string_view dmb_path) {
  ifstream f(dmb_path.data());
  if (not f.is_open()) {
    logerr("Could not open {}.", styled(dmb_path, fmt::emphasis::underline));
    exit(-1);
  }

  bool first_battle_model = true;
  size_t lineno = 0;
  string line;
  unordered_map<string, battle_model> battle_models;
  battle_model current_battle_model = {};
  while (getline(f, line)) {
    ++lineno;

    smatch m;
    if (regex_search(line, m, regex("^type\\s+([\\w\\d_]+)"))) {
      if (not first_battle_model) {
        assert(not current_battle_model.dictionary.empty());
        assert(current_battle_model.lineno != 0);
        assert(not battle_models.contains(current_battle_model.dictionary));
        battle_models[current_battle_model.dictionary] = current_battle_model;
      } else
        first_battle_model = false;
      current_battle_model = battle_model{};
      current_battle_model.lineno = lineno;
      current_battle_model.dictionary = m[1];
    } else if (regex_search(line, m,
                            regex("^texture\\s*(?:(\\w+)*[\\s,]+)*\\s*(["
                                  "\\w\\d\\.\\/]+)"))) {
      string owner = m[1];
      string path = m[2];
      if (owner.empty())
        owner = "default";
      texture t = {.lineno = lineno, .path = path};
      current_battle_model.textures[owner] = t;
    } else if (regex_search(line, m,
                            regex("^pbr_texture\\s*(?:(\\w+)*[\\s,]+)*\\s*(["
                                  "\\w\\d\\.\\/]+)"))) {
      string owner = m[1];
      string path = m[2];
      if (owner.empty())
        owner = "default";
      texture t = {.lineno = lineno, .path = path};
      current_battle_model.pbr_textures[owner] = t;
    }
  }

  // Don't forget to push the last battle_model.
  if (not current_battle_model.dictionary.empty()) {
    assert(current_battle_model.lineno != 0);
    battle_models[current_battle_model.dictionary] = current_battle_model;
  }
  if (battle_models.empty()) {
    logerr("No battle models were parsed.");
    exit(-1);
  }
  return battle_models;
}
