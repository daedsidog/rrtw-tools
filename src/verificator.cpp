#include <dcc/errno.hpp>
#include <dcc/file.hpp>
#include <dcc/logger.hpp>
#include <filesystem>
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

#define FFORMAT(file)

int main(int argc, char **argv) {
  bool no_problems = true;
  bool ignore_slave = false;
  bool check_all_referenced_textures = false;
  bool check_all_faction_textures = false;
  int problem_count = 0;
  string edu_filename = "data/export_descr_unit.txt";
  string eu_filename = "data/text/export_units.txt";
  string en_strs_filename = "data/string_overrides/en.strings";
  string dmb_filename = "data/descr_model_battle.txt";
  string root_dir = "";

  for (int i = 0; i < argc; ++i) {
    string s = argv[i];
    if (s == "--ignore-slave") {
      ignore_slave = true;
      logmsg("Will not verify slave faction.");
    } else if (s == "--check-all-faction-textures") {
      check_all_faction_textures = true;
      logmsg("Will verify all unit owners for their textures.");
    } else if (s == "--check-all-referenced-textures") {
      check_all_referenced_textures = true;
      logmsg("Will verify all referenced textures.");
    } else {
      if (i == 0)
        continue;
      fs::path p(s);
      if (fs::exists(p))
        root_dir = p.string();
    }
  }
  if (root_dir.empty()) {
    logerr("No valid mod directory given.");
    exit(-1);
  }

  logmsg("Parsing {}...", sgr::file(edu_filename));

  vector<unit> units =
      parse_units(fmt::format("{}/{}", root_dir, edu_filename));

  logmsg("Parsing {}...", sgr::file(dmb_filename));

  unordered_map<string, battle_model> battle_models =
      parse_battle_models(fmt::format("{}/{}", root_dir, dmb_filename));

  logmsg("Loading {}...", sgr::file(eu_filename));

  string export_units;
  if (freadall(fmt::format("{}/{}", root_dir, eu_filename).c_str(),
               export_units) == -1) {
    logerr("Could not read {}: {}", sgr::file(eu_filename), errmsg());
    exit(-1);
  }

  logmsg("Loading {}...", sgr::file(en_strs_filename));

  string en_strings;
  if (freadall(fmt::format("{}/{}", root_dir, en_strs_filename).c_str(),
               en_strings) == -1) {
    logerr("Could not read {}: {}.", sgr::file(en_strs_filename), errmsg());
    exit(-1);
  }

  logmsg("Verifying units...");
  for (const unit &u : units) {
    vector<string> problems;
    vector<string> str_entries = {u.dictionary,
                                  fmt::format("{}_descr", u.dictionary),
                                  fmt::format("{}_descr_short", u.dictionary)};
    smatch m;

    // Verify export_units.txt
    for (const auto &s : str_entries) {
      if (not regex_search(export_units, m,
                           regex(fmt::format("(\\{{{}\\}})", s))))
        problems.push_back(fmt::format("{} missing from {}.",
                                       sgr::problem(fmt::format("{{{}}}", s)),
                                       sgr::file(eu_filename)));
    }

    // Verify en.strings
    for (const auto &s : str_entries) {
      if (not regex_search(en_strings, m,
                           regex(fmt::format("\"Rome\\.Override\\.{}\"", s))))
        problems.push_back(
            fmt::format("{} missing from {}.",
                        sgr::problem(fmt::format("Rome.Override.{}", s)),
                        sgr::file(en_strs_filename)));
    }

    // Verify unit cards
    if (u.mercenary) {

      // This is a mercenary unit, so we should verify it has unit cards for the
      // mercenary faction only.
      string unit_card = fmt::format("#{}.tga", u.dictionary);
      if (not fs::exists(
              fmt::format("{}/data/ui/units/mercs/{}", root_dir, unit_card)))
        problems.push_back(fmt::format("{} missing from {}.",
                                       sgr::problem(unit_card),
                                       sgr::file("ui/units/mercs")));
      unit_card = fmt::format("{}_info.tga", u.dictionary);
      if (not fs::exists(
              fmt::format("{}/data/ui/unit_info/merc/{}", root_dir, unit_card)))
        problems.push_back(fmt::format("{} missing from {}.",
                                       sgr::problem(unit_card),
                                       sgr::file("ui/unit_info/merc")));
    } else {
      if (u.owners.empty())
        problems.push_back("This unit has no owners.");
      for (const auto &owner : u.owners) {
        if (ignore_slave and owner == "slave")
          continue;

        string unit_card = fmt::format("#{}.tga", u.dictionary);
        if (not fs::exists(fmt::format("{}/data/ui/units/{}/{}", root_dir,
                                       owner, unit_card)))
          problems.push_back(
              fmt::format("{} missing from {}.", sgr::problem(unit_card),
                          sgr::file(fmt::format("ui/units/{}", owner))));
        unit_card = fmt::format("{}_info.tga", u.dictionary);
        if (not fs::exists(fmt::format("{}/data/ui/unit_info/{}/{}", root_dir,
                                       owner, unit_card)))
          problems.push_back(
              fmt::format("{} missing from {}.", sgr::problem(unit_card),
                          sgr::file(fmt::format("ui/unit_info/{}", owner))));
      }
    }

    // Verify textures
    unordered_set<string> missing_textures;
    auto verify_troop_textures = [&battle_models, &problems, &u,
                                  &missing_textures, dmb_filename, ignore_slave,
                                  check_all_faction_textures,
                                  check_all_referenced_textures,
                                  root_dir](vector<string> troops) -> void {
      for (const auto &soldier : troops) {
        if (not battle_models.contains(soldier)) {
          problems.push_back(fmt::format("{} missing from {}.",
                                         sgr::problem(soldier),
                                         sgr::file(dmb_filename)));
          continue;
        }

        const auto &textures = battle_models[soldier].textures;
        const auto &pbr_textures = battle_models[soldier].pbr_textures;
        if (not pbr_textures.contains("default"))
          problems.push_back(fmt::format(
              "Missing default pbr_texture for {} at {}.",
              sgr::semiunique(soldier),
              sgr::file(fmt::format("{}:{}", dmb_filename,
                                    battle_models[soldier].lineno))));
        if (not textures.contains("default"))
          problems.push_back(fmt::format(
              "Missing default texture for {} at {}.", sgr::semiunique(soldier),
              sgr::file(fmt::format("{}:{}", dmb_filename,
                                    battle_models[soldier].lineno))));
        if (check_all_faction_textures) {
          for (const auto &owner : u.owners) {
            if (ignore_slave and owner == "slave")
              continue;
            if (not pbr_textures.contains(owner))
              problems.push_back(fmt::format(
                  "Missing pbr_texture for {} for {} at {}.",
                  sgr::semiunique(soldier), sgr::unique(owner),
                  sgr::file(fmt::format("{}:{}", dmb_filename,
                                        battle_models[soldier].lineno))));
            if (not textures.contains(owner))
              problems.push_back(fmt::format(
                  "Missing texture for {} for {} at {}.",
                  sgr::semiunique(soldier), sgr::unique(owner),
                  sgr::file(fmt::format("{}:{}", dmb_filename,
                                        battle_models[soldier].lineno))));
          }
        }

        auto check_disk_for_textures =
            [&problems, &soldier, &missing_textures, dmb_filename,
             check_all_referenced_textures,
             root_dir](unordered_map<string, texture> textures) {
              for (const auto &[owner, texture] : textures) {
                if (not check_all_referenced_textures and owner != "default")
                  continue;

                // For some reason, the game's files reference by one extension,
                // while the files exist on disk by another.
                string actual_path = fmt::format("{}.dds", texture.path);
                if (not fs::exists(
                        fmt::format("{}/{}", root_dir, actual_path)) and
                    not missing_textures.contains(actual_path)) {
                  problems.push_back(fmt::format(
                      "Texture {} missing from path for {} at {}.",
                      sgr::file(actual_path), sgr::semiunique(soldier),
                      sgr::file(
                          fmt::format("{}:{}", dmb_filename, texture.lineno))));
                  missing_textures.insert(actual_path);
                }
              }
            };
        check_disk_for_textures(pbr_textures);
        check_disk_for_textures(textures);
      }
    };
    verify_troop_textures(u.soldiers);
    verify_troop_textures(u.officers);

    if (not problems.empty()) {
      no_problems = false;
      ++problem_count;
      fmt::print(stderr, "\n{}) {} at {}:\n", problem_count,
                 sgr::unique(u.dictionary),
                 sgr::file(fmt::format("{}:{}", edu_filename, u.lineno)));
      for (size_t i = 0; i < problems.size(); ++i)
        fmt::print(stderr, "\t {}. {}{}\n", i + 1,
                   string(int(log10(problems.size())) - int(log10(i + 1)), ' '),
                   problems[i]);
    }
  }
  if (no_problems)
    logmsg("All {} units are valid.", sgr::semiunique(units.size()));
  fmt::print("\n");
  return 0;
}
