# daedsidog-tools
This directory contains various tools (currently just one) requested by RIS modders. If you have a tool request that would help you with mundane or tedious tasks, please feel free to contact me (International Man of Mystery) on the RIS discord. I am not an experienced modder, so please be as descriptive as possible with what you want your tool to accomplish.

# Usage
The tools should work straight out the box for Windows users. Simply run the `.bat` file of each respective script. The scripts call executables located in the `bin` directory, which more advanced users may leverage by running those executables directly from the command-line.

Please note  that I won't be providing any documentation on how to work directly with the binaries, so the only usage insight (with the **binaries**) is inside the scripts.

# Tools
## verify_units
Verifies that units described in `export_descr_unit.txt` have no missing unit cards, textures, models, or text.

## verify-units-ignore-slave
Same as [verify-units](##verify-units), except it does not check the unit for the slave faction.

## verify-units-check-all-referenced-paths
Same as [verify-units](##verify-units), except it will also verify other referenced paths listed in
`descr_battle_model.txt` and see they exist as specified.

## verify-units-check-all-factions
Same as [verify-units-check-all-referenced-paths](##verify-units-check-all-referenced-paths), except it will go over **all** of the unit's owners (listed in `export_descr_unit.txt`), instead of just verifying the default and referenced paths only. 

## verify-characters
Verifies characters similarly to [verify-units](##verify-units).

## verify-characters-check-all-referenced-paths
Verifies characters similarly to [verify-units-check-all-referenced-paths](##verify-units-check-all-referenced-paths).

## verify-characters-check-all-factions
Verifies characters similarly to [verify-units-check-all-factions](##verify-units-check-all-factions).

# verify-banners
Verifies textures referenced in `descr_banners.txt`.

## generate_export_units
Creates a full `data/text/export_units.txt` file from the entries in `data/export_descr_unit.txt`.