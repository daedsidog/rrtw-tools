#include "common.hpp"

#include <cassert>
#include <cstdio>
#include <dcc/file.hpp>
#include <dcc/logger.hpp>
#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_set>

using namespace std;
using namespace dcc;

class unit_parser : public parser<unit> {
public:
  unit_parser(string_view path) : parser(path) {
    partition = "dictionary";
    comment = ";";
    entries["dictionary"] = [this]() {
      t.lineno = lineno();
      t.dictionary = entry();
    };

    entries["attributes"] = [this]() {
      t.attributes = strtok(entry());
      for (const auto& attr : t.attributes) {
        if (attr == "mercenary_unit")
          t.mercenary = true;
      }
    };
    entries["ownership"] = [this]() { t.owners = strtok(entry()); };
    entries["officer"] = [this]() { t.officers.push_back(entry()); };
    entries["soldier"] = [this]() { t.soldiers.push_back(strtok(entry())[0]); };
    sets["soldiers"] = [this]() {
      const vector<string> soldiers = strtok(set().nested_sets[0].entries);
      t.soldiers.insert(t.soldiers.end(), soldiers.begin(), soldiers.end());
    };
  };
};

class battle_model_parser : public parser<battle_model, string> {
public:
  battle_model_parser(string_view path) : parser(path) {
    partition = "type";
    comment = ";";
    entries["type"] = [this]() {
      t.dictionary = entry();
      t.lineno = lineno();
    };
    entries["texture"] = [this]() {
      vector<string> owner_and_path = strtok(entry());
      texture tex;
      tex.lineno = lineno();
      if (owner_and_path.size() == 1) {
        tex.path = entry();
        t.textures["default"] = tex;
      } else {
        tex.path = owner_and_path[1];
        t.textures[owner_and_path[0]] = tex;
      }
    };
    entries["pbr_texture"] = [this]() {
      vector<string> owner_and_path = strtok(entry());
      texture tex;
      tex.lineno = lineno();
      if (owner_and_path.size() == 1) {
        tex.path = entry();
        t.pbr_textures["default"] = tex;
      } else {
        tex.path = owner_and_path[1];
        t.pbr_textures[owner_and_path[0]] = tex;
      }
    };
    key = [this]() { return t.dictionary; };
  };
};

vector<unit> parse_units(string_view edu_path) {
  unit_parser p(edu_path);
  vector<unit> units;
  if (p.parse(units) == -1) {
    dcc_logerr("Could not parse {}.", sgr::file(edu_path));
    exit(-1);
  }
  return units;
}

unordered_map<string, battle_model> parse_battle_models(string_view dmb_path) {
  battle_model_parser p(dmb_path);
  unordered_map<string, battle_model> battle_models;
  if (p.parse(battle_models) == -1) {
    dcc_logerr("Could not parse {}.", sgr::file(dmb_path));
    exit(-1);
  }
  return battle_models;
}
