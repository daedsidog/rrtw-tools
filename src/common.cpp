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
      }
      else {
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
      }
      else {
        tex.path = owner_and_path[1];
        t.pbr_textures[owner_and_path[0]] = tex;
      }
    };
    entries["model_flexi"] = [this]() {
      t.model_paths.insert(strtok(entry())[0]);
    };
    entries["model_flexi_m"] = [this]() {
      t.model_paths.insert(strtok(entry())[0]);
    };
    entries["no_variation model_flexi"] = [this]() {
      t.model_paths.insert(strtok(entry())[0]);
    };
    entries["no_variation model_flexi_m"] = [this]() {
      t.model_paths.insert(strtok(entry())[0]);
    };
    key = [this]() { return t.dictionary; };
  };
};

class character_parser : public parser<strat_model_entry, string> {
public:
  character_parser(string_view path) : parser(path) {
    partition = "type";
    comment = ";";
    entries["type"] = [this]() {
      t.type = entry();
      t.lineno = lineno();
    };
    entries["faction"] = [this]() { t.last_faction = entry(); };
    entries["strat_model"] = [this]() { t.models[t.last_faction] = entry(); };
    entries["strat_card"] = [this]() {
      t.strat_cards[t.last_faction] = entry();
    };
  };
};

class strat_model_parser : public parser<strat_model, string> {
public:
  strat_model_parser(string_view path) : parser(path) {
    partition = "type";
    comment = ";";
    entries["type"] = [this]() {
      t.lineno = lineno();
      t.type = entry();
    };
    entries["model_flexi"] = [this]() { t.path = strtok(entry())[0]; };
    entries["no_variation model_flexi"] = [this]() {
      t.nv_path = strtok(entry())[0];
    };
    entries["texture"] = [this]() {
      vector<string> owner_and_path = strtok(entry());
      texture tex;
      tex.lineno = lineno();
      if (owner_and_path.size() == 1) {
        tex.path = entry();
        t.textures["default"] = tex;
      }
      else {
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
      }
      else {
        tex.path = owner_and_path[1];
        t.pbr_textures[owner_and_path[0]] = tex;
      }
    };
    key = [this]() { return t.type; };
  };
};

class banner_parser : public parser<banner> {
public:
  banner_parser(string_view path) : parser(path) {
    partition = "banner";
    comment = ";";
    entries["banner"] = [this]() {
      t.type = entry();
      t.lineno = lineno();
    };
    entries["standard_texture"] = [this]() {
      t.texture_paths.insert(fmt::format("data/{}.dds", entry()));
    };
    entries["rebels_texture"] = [this]() {
      t.texture_paths.insert(fmt::format("data/{}.dds", entry()));
    };
    entries["ally_texture"] = [this]() {
      t.texture_paths.insert(fmt::format("data/{}.dds", entry()));
    };
    entries["routing_texture"] = [this]() {
      t.texture_paths.insert(fmt::format("data/{}.dds", entry()));
    };
  }
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

unordered_map<string, strat_model> parse_strat_models(string_view dms_path) {
  strat_model_parser p(dms_path);
  unordered_map<string, strat_model> strat_models;
  if (p.parse(strat_models) == -1) {
    dcc_logerr("Could not parse {}.", sgr::file(dms_path));
    exit(-1);
  }
  return strat_models;
}

vector<strat_model_entry> parse_strat_model_entries(string_view dc_path) {
  character_parser p(dc_path);
  vector<strat_model_entry> v;
  if (p.parse(v) == -1) {
    dcc_logerr("Could not parse {}.", sgr::file(dc_path));
    exit(-1);
  }
  return v;
}

vector<banner> parse_banners(string_view db_path) {
  banner_parser p(db_path);
  vector<banner> v;
  if (p.parse(v) == -1) {
    dcc_logerr("Could not parse {}.", sgr::file(db_path));
    exit(-1);
  }
  return v;
}
