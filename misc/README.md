# daedsidog-tools
This directory contains various tools (currently just one) requested by RIS modders. If you have a tool request that would help you with mundane or tedious tasks, please feel free to contact me (International Man of Mystery) on the RIS discord. I am not an experienced modder, so please be as descriptive as possible with what you want your tool to accomplish.

 * [verify_units](##verify_units)
 * [verify_units_ignore_slave](##verify_units_ignore_slave)
 * [verify_units_check_all_referenced_textures](##verify_units_check_all_referenced_textures)
 * [verify_units_check_all_fraction_textures](##verify_units_check_all_fraction_textures)

# Usage
The tools should work straight out the box for Windows users. Simply run the `.bat` file of each respective script. The scripts call executables located in the `bin` directory, which more advanced users may leverage by running those executables directly from the command-line.

Please note  that I won't be providing any documentation on how to work directly with the binaries, so the only usage insight (with the **binaries**) is inside the scripts.

# Tools
## verify_units
Verifies that units described in `export_descr_unit.txt` have no missing unit cards, textures, or text.

## verify_units_ignore_slave
Same as [verify_units](##verify_units), except it does not check the unit cards for the slave faction.

## verify_units_check_all_referenced_textures
Same as [verify_units](##verify_units), except it will also verify other referenced textures listed in
`descr_battle_model.txt` and see they exist in the specified path.

## verify_units_check_all_fraction_textures
Same as [verify_units_check_all_referenced_textures](##verify_units_check_all_referenced_textures), except it will go over **all** of the unit's owners (listed in `export_descr_unit.txt`), instead of just verifying the default and referenced textures. 

# Source
You can view the tools-specific source code [here](https://github.com/daedsidog/rrtw-tools). I made use of my own private library in the making of this which I have not included, so you cannot compile this by yourself as it is without replacing the implementation.