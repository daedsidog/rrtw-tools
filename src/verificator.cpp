#include <dcc/errno.hpp>
#include <dcc/file.hpp>
#include <dcc/logger.hpp>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string_view>
#include <unordered_set>

#include "common.hpp"

using namespace std;
using namespace dcc;
namespace fs = std::filesystem;

#ifndef ROOT_MOD_DIR
#define ROOT_MOD_DIR "../../RIS"
#endif

namespace g {

  const char commsym = (char)172;
  const string_view edu_filename = "data/export_descr_unit.txt";
  const string_view eu_filename = "data/text/export_units.txt";
  const string_view en_strs_filename = "data/string_overrides/en.strings";
  const string_view dmb_filename = "data/descr_model_battle.txt";
  const string_view dc_filename = "data/descr_character.txt";
  const string_view db_filename = "data/descr_banners.txt";
  const string_view dms_filename = "data/descr_model_strat.txt";
  bool check_all_factions = false;
  bool check_all_referenced_paths = false;
  bool ignore_slave = false;
  bool generate_export_units = false;
  bool verify_characters = false;
  bool verify_banners = false;
  bool no_problems = true;
  int problem_count = 0;
  string root_dir = "";

}; // namespace g

void verify_banners() {
  dcc_logmsg("Parsing {}...", sgr::file(g::db_filename));

  vector<banner> banners = parse_banners(g::db_filename);
  dcc_logmsg("Verifying banners...");
  for (const auto& ban : banners) {
    vector<string> problems;
    for (const auto& texpath : ban.texture_paths) {
      if (not fs::exists(texpath))
        problems.push_back(
          fmt::format("Texture {} missing from path.", sgr::file(texpath)));
    }
    if (not problems.empty()) {
      g::no_problems = false;
      ++g::problem_count;
      flogmsg(stderr, "", "\n{} {} at {}:",
              fmt::format(fg(fmt::color::white), "{})", g::problem_count),
              sgr::unique(ban.type),
              sgr::file(fmt::format("{}:{}", g::db_filename, ban.lineno)));
      for (size_t i = 0; i < problems.size(); ++i)
        flogmsg(stderr, "", "\t {} {}{}",
                fmt::format(fmt::fg(fmt::color::white), "{}.", i + 1),
                string(int(log10(problems.size())) - int(log10(i + 1)), ' '),
                problems[i]);
    }
  }
  if (g::no_problems)
    dcc_logmsg("All {} banners are valid.", sgr::semiunique(banners.size()));
}

void verify_strat_models() {
  dcc_logmsg("Parsing {}...", sgr::file(g::dc_filename));

  vector<strat_model_entry> strat_model_entries;
  strat_model_entries = parse_strat_model_entries(g::dc_filename);
  dcc_logmsg("Parsing {}...", sgr::file(g::dms_filename));

  unordered_map<std::string, strat_model> strat_models;
  strat_models = parse_strat_models(g::dms_filename);
  dcc_logmsg("Verifying characters...");
  for (const auto& entry : strat_model_entries) {
    unordered_set<string> handled_models;
    vector<string> problems;

    // // strat_cards
    // for (const auto& [owner, scstr] : entry.strat_cards) {
    //   if (owner == "slave" and g::ignore_slave)
    //     continue;
    //   if (not fs::exists(scstr)) {
    //     problems.push_back(
    //       fmt::format("strat_card {} missing from path for {}.",
    //                   sgr::file(scstr), sgr::unique(owner)));
    //   }
    // }
    for (const auto& [owner, modelstr] : entry.models) {
      if (owner == "slave" and g::ignore_slave)
        continue;
      if (not strat_models.contains(modelstr)) {
        problems.push_back(fmt::format("No entry for {} found in {}.",
                                       sgr::semiunique(modelstr),
                                       sgr::file(g::dms_filename)));
      }
      else {
        if (handled_models.contains(modelstr))
          continue;
        handled_models.insert(modelstr);

        bool got_default_pbr_tex = true;
        bool got_default_tex = true;
        auto& sm = strat_models.at(modelstr);
        auto ddspath = [](const string_view tpath) {
          return fmt::format("{}.dds", tpath);
        };

        // PBR textures
        if (not sm.pbr_textures.contains("default")) {
          problems.push_back(fmt::format(
            "Missing default pbr_texture for {} at {}.",
            sgr::semiunique(modelstr),
            sgr::file(fmt::format("{}:{}", g::dms_filename, sm.lineno))));
          got_default_pbr_tex = false;
        }
        else if (not fs::exists(ddspath(sm.pbr_textures.at("default").path))) {
          texture& t = sm.pbr_textures.at("default");
          problems.push_back(fmt::format(
            "Default texture {} for {} is missing from path at {}.",
            sgr::file(ddspath(t.path)), sgr::semiunique(modelstr),
            sgr::file(fmt::format("{}:{}", g::dms_filename, t.lineno))));
          got_default_pbr_tex = false;
        }
        if (not sm.pbr_textures.contains(owner)) {
          if (not got_default_pbr_tex or g::check_all_factions) {
            problems.push_back(fmt::format(
              "Missing {} pbr_texture at for {} at {}.", sgr::unique(owner),
              sgr::semiunique(modelstr),
              sgr::file(fmt::format("{}:{}", g::dms_filename, sm.lineno))));
          }
        }
        else if (not got_default_pbr_tex or g::check_all_referenced_paths) {
          texture& t = sm.textures.at(owner);
          if (not fs::exists(ddspath(t.path))) {
            problems.push_back(fmt::format(
              "Texture {} for {} for {} missing from path at {}.",
              sgr::file(ddspath(t.path)), sgr::semiunique(modelstr),
              sgr::unique(owner),
              sgr::file(fmt::format("{}:{}", g::dms_filename, t.lineno))));
          }
        }

        // Regular textures
        if (not sm.textures.contains("default")) {
          problems.push_back(fmt::format(
            "Missing default texture for {} at {}.", sgr::semiunique(modelstr),
            sgr::file(fmt::format("{}:{}", g::dms_filename, sm.lineno))));
          got_default_tex = false;
        }
        else if (not fs::exists(ddspath(sm.textures.at("default").path))) {
          texture& t = sm.textures.at("default");
          problems.push_back(fmt::format(
            "Default texture {} for {} is missing from path at {}.",
            sgr::file(ddspath(t.path)), sgr::semiunique(modelstr),
            sgr::file(fmt::format("{}:{}", g::dms_filename, t.lineno))));
          got_default_tex = false;
        }
        if (not sm.textures.contains(owner)) {
          if (not got_default_tex or g::check_all_factions) {
            problems.push_back(fmt::format(
              "Missing {} texture at for {} at {}.", sgr::unique(owner),
              sgr::semiunique(modelstr),
              sgr::file(fmt::format("{}:{}", g::dms_filename, sm.lineno))));
          }
        }
        else if (not got_default_tex or g::check_all_referenced_paths) {
          texture& t = sm.textures.at(owner);
          if (not fs::exists(ddspath(t.path))) {
            problems.push_back(fmt::format(
              "Texture {} for {} for {} missing from path at {}.",
              sgr::file(ddspath(t.path)), sgr::semiunique(modelstr),
              sgr::unique(owner),
              sgr::file(fmt::format("{}:{}", g::dms_filename, t.lineno))));
          }
        }

        // Models
        if (sm.path.empty()) {
          problems.push_back(fmt::format(
            "Missing model_flexi for {} at {}.", sgr::semiunique(modelstr),
            sgr::file(fmt::format("{}:{}", g::dms_filename, sm.lineno))));
        }
        else if (not fs::exists(sm.path)) {
          problems.push_back(fmt::format(
            "Model {} is missing from path at {}.", sgr::file(sm.path),
            sgr::file(fmt::format("{}:{}", g::dms_filename, sm.lineno))));
        }
        if (sm.nv_path.empty()) {
          problems.push_back(fmt::format(
            "Missing no_variation model_flexi for {} at {}.",
            sgr::unique(modelstr),
            sgr::file(fmt::format("{}:{}", g::dms_filename, sm.lineno))));
        }
        else if (sm.nv_path != sm.path and not fs::exists(sm.nv_path)) {
          problems.push_back(fmt::format(
            "Model {} is missing from path at {}.", sgr::file(sm.nv_path),
            sgr::file(fmt::format("{}:{}", g::dms_filename, sm.lineno))));
        }
      }
    }

    if (not problems.empty()) {
      g::no_problems = false;
      ++g::problem_count;
      flogmsg(stderr, "", "\n{} {} at {}:",
              fmt::format(fg(fmt::color::white), "{})", g::problem_count),
              sgr::unique(entry.type),
              sgr::file(fmt::format("{}:{}", g::dc_filename, entry.lineno)));
      for (size_t i = 0; i < problems.size(); ++i)
        flogmsg(stderr, "", "\t {} {}{}",
                fmt::format(fmt::fg(fmt::color::white), "{}.", i + 1),
                string(int(log10(problems.size())) - int(log10(i + 1)), ' '),
                problems[i]);
    }
  }
  if (g::no_problems)
    dcc_logmsg("All {} character models are valid.",
               sgr::semiunique(strat_models.size()));
}

void generate_export_units(string_view progname) {
  dcc_logmsg("Parsing {}...", sgr::file(g::edu_filename));

  vector<unit> units = parse_units(g::edu_filename);
  ofstream eu(g::eu_filename.data(), ios::binary);
  string eustr;
  if (not eu) {
    dcc_logerr("Could not open {} for writing: {}.", sgr::file(g::eu_filename),
               errmsg());
    exit(-1);
  }
  dcc_logmsg("Generating {}...", sgr::file(g::eu_filename));
  eustr +=
    fmt::format("{} File autogenerated by {}.\n\n", g::commsym, progname);
  eustr += fmt::format("{} Text used for building names and descriptions\n",
                       g::commsym);
  eustr += fmt::format("{} Lines that begin with this character are comments\n",
                       g::commsym);
  eustr +=
    fmt::format("{} and should not be translated or altered\n", g::commsym);
  eustr += fmt::format(
    "{} Items inside curly brackets are tags and should not be translated\n",
    g::commsym);
  eustr += fmt::format("{} The text following each tag on the same, or next "
                       "line does need to be translated\n",
                       g::commsym);
  for (const auto& unit : units) {
    eustr += "\n\n";
    eustr += fmt::format("{{{}}}{}\n", unit.dictionary, unit.dictionary);
    eustr +=
      fmt::format("{{{}_descr}}{}_descr\n", unit.dictionary, unit.dictionary);
    eustr += fmt::format("{{{}_descr_short}}{}_descr_short", unit.dictionary,
                         unit.dictionary);
  }

  // This file needs to be in UTF16LE.
  // This is a very primitive solution, and I am in no way proud of it, but my
  // quest in leveraging the standard library for mystic conversion between
  // encodings ended poorly, so I just did it the only fashioned way.
  char nullbyte = 0x0;
  wchar_t bom = 0xfeff; // UTF16LE BOM
  eu.write((char*)&bom, sizeof(wchar_t));
  for (size_t i = 0, j = 0; i != eustr.length() - 1; ++j) {
    if (j % 2 == 0) {
      eu.write((char*)(eustr.data() + i), sizeof(char));
      ++i;
    }
    else
      eu.write((char*)&nullbyte, sizeof(char));
  }
  dcc_logmsg("Finished generating {}.", sgr::file(g::eu_filename));
}

void verify_units() {
  dcc_logmsg("Parsing {}...", sgr::file(g::edu_filename));

  vector<unit> units = parse_units(g::edu_filename);

  dcc_logmsg("Parsing {}...", sgr::file(g::dmb_filename));

  unordered_map<string, battle_model> battle_models =
    parse_battle_models(g::dmb_filename);

  dcc_logmsg("Loading {}...", sgr::file(g::eu_filename));

  string export_units;
  if (freadall(g::eu_filename.data(), export_units) == -1) {
    dcc_logerr("Could not read {}: {}", sgr::file(g::eu_filename), errmsg());
    exit(-1);
  }

  string en_strings;
  if (freadall(g::en_strs_filename.data(), en_strings) == -1) {
    dcc_logerr("Could not read {}: {}.", sgr::file(g::en_strs_filename),
               errmsg());
    exit(-1);
  }

  dcc_logmsg("Verifying units...");
  for (const unit& u : units) {
    vector<string> problems;
    vector<string> str_entries = {u.dictionary,
                                  fmt::format("{}_descr", u.dictionary),
                                  fmt::format("{}_descr_short", u.dictionary)};
    smatch m;

    // Verify export_units.txt
    for (const auto& s : str_entries) {
      if (not regex_search(export_units, m,
                           regex(fmt::format("(\\{{{}\\}})", s))))
        problems.push_back(fmt::format("{} missing from {}.",
                                       sgr::problem(fmt::format("{{{}}}", s)),
                                       sgr::file(g::eu_filename)));
    }

    // Verify en.strings
    for (const auto& s : str_entries) {
      if (not regex_search(en_strings, m,
                           regex(fmt::format("\"Rome\\.Override\\.{}\"", s))))
        problems.push_back(
          fmt::format("{} missing from {}.",
                      sgr::problem(fmt::format("Rome.Override.{}", s)),
                      sgr::file(g::en_strs_filename)));
    }

    // Verify unit cards
    if (u.mercenary) {

      // This is a mercenary unit, so we should verify it has unit cards for
      // the mercenary faction only.
      string unit_card = fmt::format("#{}.tga", u.dictionary);
      if (not fs::exists(fmt::format("data/ui/units/mercs/{}", unit_card)))
        problems.push_back(fmt::format("{} missing from {}.",
                                       sgr::problem(unit_card),
                                       sgr::file("ui/units/mercs")));
      unit_card = fmt::format("{}_info.tga", u.dictionary);
      if (not fs::exists(fmt::format("data/ui/unit_info/merc/{}", unit_card)))
        problems.push_back(fmt::format("{} missing from {}.",
                                       sgr::problem(unit_card),
                                       sgr::file("data/ui/unit_info/merc")));
    }
    else {
      if (u.owners.empty())
        problems.push_back("This unit has no owners.");
      for (const auto& owner : u.owners) {
        if (g::ignore_slave and owner == "slave")
          continue;

        string unit_card = fmt::format("#{}.tga", u.dictionary);
        if (not fs::exists(
              fmt::format("data/ui/units/{}/{}", owner, unit_card)))
          problems.push_back(
            fmt::format("{} missing from {}.", sgr::problem(unit_card),
                        sgr::file(fmt::format("data/ui/units/{}", owner))));
        unit_card = fmt::format("{}_info.tga", u.dictionary);
        if (not fs::exists(
              fmt::format("data/ui/unit_info/{}/{}", owner, unit_card)))
          problems.push_back(
            fmt::format("{} missing from {}.", sgr::problem(unit_card),
                        sgr::file(fmt::format("data/ui/unit_info/{}", owner))));
      }
    }

    unordered_set<string> missing_textures;
    unordered_set<string> handled_troops;
    auto verify_bm = [&handled_troops, &battle_models, &problems, &u,
                      &missing_textures](vector<string> troops) -> void {
      for (const auto& soldier : troops) {
        if (handled_troops.contains(soldier))
          continue;
        handled_troops.insert(soldier);
        if (not battle_models.contains(soldier)) {
          problems.push_back(fmt::format("{} missing from {}.",
                                         sgr::problem(soldier),
                                         sgr::file(g::dmb_filename)));
          continue;
        }

        // Verify models
        for (const auto& mpath : battle_models.at(soldier).model_paths) {
          if (not fs::exists(mpath)) {
            problems.push_back(fmt::format(
              "Model {} missing from path for {} at {}.", sgr::file(mpath),
              sgr::semiunique(soldier),
              sgr::file(fmt::format("{}:{}", g::dmb_filename,
                                    battle_models.at(soldier).lineno))));
          }
        }

        // Verify textures
        const auto& textures = battle_models[soldier].textures;
        const auto& pbr_textures = battle_models[soldier].pbr_textures;
        if (not pbr_textures.contains("default")) {
          problems.push_back(
            fmt::format("Missing default pbr_texture for {} at {}.",
                        sgr::semiunique(soldier),
                        sgr::file(fmt::format("{}:{}", g::dmb_filename,
                                              battle_models[soldier].lineno))));
        }
        if (not textures.contains("default")) {
          problems.push_back(fmt::format(
            "Missing default texture for {} at {}.", sgr::semiunique(soldier),
            sgr::file(fmt::format("{}:{}", g::dmb_filename,
                                  battle_models[soldier].lineno))));
        }
        if (g::check_all_factions) {
          for (const auto& owner : u.owners) {
            if (g::ignore_slave and owner == "slave")
              continue;
            if (not pbr_textures.contains(owner))
              problems.push_back(fmt::format(
                "Missing pbr_texture for {} for {} at {}.",
                sgr::semiunique(soldier), sgr::unique(owner),
                sgr::file(fmt::format("{}:{}", g::dmb_filename,
                                      battle_models[soldier].lineno))));
            if (not textures.contains(owner))
              problems.push_back(fmt::format(
                "Missing texture for {} for {} at {}.",
                sgr::semiunique(soldier), sgr::unique(owner),
                sgr::file(fmt::format("{}:{}", g::dmb_filename,
                                      battle_models[soldier].lineno))));
          }
        }

        auto check_disk_for_textures =
          [&problems, &soldier,
           &missing_textures](unordered_map<string, texture> textures) {
            for (const auto& [owner, texture] : textures) {
              if (not g::check_all_referenced_paths and owner != "default")
                continue;

              // For some reason, the game's files reference by one extension,
              // while the files exist on disk by another.
              string actual_path = fmt::format("{}.dds", texture.path);
              if (not fs::exists(actual_path) and
                  not missing_textures.contains(actual_path)) {
                problems.push_back(
                  fmt::format("Texture {} missing from path for {} at {}.",
                              sgr::file(actual_path), sgr::semiunique(soldier),
                              sgr::file(fmt::format("{}:{}", g::dmb_filename,
                                                    texture.lineno))));
                missing_textures.insert(actual_path);
              }
            }
          };
        check_disk_for_textures(pbr_textures);
        check_disk_for_textures(textures);
      }
    };
    verify_bm(u.soldiers);
    verify_bm(u.officers);

    if (not problems.empty()) {
      g::no_problems = false;
      ++g::problem_count;
      flogmsg(stderr, "", "\n{} {} at {}:",
              fmt::format(fg(fmt::color::white), "{})", g::problem_count),
              sgr::unique(u.dictionary),
              sgr::file(fmt::format("{}:{}", g::edu_filename, u.lineno)));
      for (size_t i = 0; i < problems.size(); ++i)
        flogmsg(stderr, "", "\t {} {}{}",
                fmt::format(fmt::fg(fmt::color::white), "{}.", i + 1),
                string(int(log10(problems.size())) - int(log10(i + 1)), ' '),
                problems[i]);
    }
  }
  if (g::no_problems)
    dcc_logmsg("All {} units are valid.", sgr::semiunique(units.size()));
}

int main(int argc, char** argv) {
  for (int i = 0; i < argc; ++i) {
    string s = argv[i];
    if (s == "--ignore-slave")
      g::ignore_slave = true;
    else if (s == "--check-all-factions") {
      g::check_all_factions = true;
      g::check_all_referenced_paths = true;
    }
    else if (s == "--check-all-referenced-paths")
      g::check_all_referenced_paths = true;
    else if (s == "--generate-export-units")
      g::generate_export_units = true;
    else if (s == "--verify-characters")
      g::verify_characters = true;
    else if (s == "--verify-banners")
      g::verify_banners = true;
    else {
      if (i == 0)
        continue;
      fs::path p(s);
      if (fs::exists(p))
        g::root_dir = p.string();
    }
  }
  if (g::root_dir.empty()) {
    dcc_logerr("No valid mod directory given.");
    exit(-1);
  }

  fs::current_path(g::root_dir);

  auto print_flag_info = []() {
    if (g::check_all_factions)
      dcc_loginf("Will verify all entry owners for their textures.");
    else if (g::check_all_referenced_paths)
      dcc_loginf("Will verify all referenced textures.");
    else if (g::ignore_slave)
      dcc_loginf("Will not verify slave faction.");
  };

  if (g::verify_characters) {
    print_flag_info();
    verify_strat_models();
  }
  else if (g::verify_banners)
    verify_banners();
  else if (g::generate_export_units)
    generate_export_units(argv[0]);
  else {
    print_flag_info();
    verify_units();
  }

  return 0;
}
