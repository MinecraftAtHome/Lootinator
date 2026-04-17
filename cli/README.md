# Lootinator CLI Usage Guide

## About the project

Lootinator lets you find *loot seeds* - a type of internal seed that Minecraft uses to determine chest loot.

Lootinator does not find loot seeds directly, it instead generates cuda kernels which can in turn be compiled and run on any
NVIDIA graphics card.

## Quick Start

- In order to run the CLI, you will need the json of the loot table you are targetting. These can be found for all versions here: https://mcasset.cloud/latest

- In addition, you will need a constraint file describing all the constraints you are targetting. The json schematic is as follows:
```
[
    {
        // the name of the item you are targetting, must be prefixed with minecraft:
        "item": <item_name>,

        // the minimum and maximum item count for this constraint
        "range": { 
            "min": <item_min>",
            "max": <item_max>
        },

        // the slot of the item between 0-27 (currently ignored)
        "slot": <slot_index>, 

        // optional attribute data for enchantments
		"attributes": [ 
			{
                "type": <enchant_name>, // e.g. efficiency
                "level": <enchant_level> // optional
            }
		]
    }
]
```

- With both of these json files, we can call the lootinator-cli

```bash
./lootinator-cli --loot-table <file_path.json> --constraint-file <file_path.json> -o <filepath> [-sc] [-sk] [-v <version>]
```

- The options are as follows
```
-sc, --seedcracking 
    if enabled, the kernel will only accept the **exact** chest contents specified in the constraint file

-sk, --single-kernel
    generates the predicted best kernel rather than benchmarking all possible kernels

-v, --version
    the target minecraft version e.g. 1.16 or latest for the latest supported release
```

- Once you have run the cli, you will be left with an output cuda (.cu) file

## Usage example

- Target loot table: https://mcasset.cloud/26.1.2/data/minecraft/loot_table/chests/ruined_portal.json

- Constraints file - `constraints_4_notches.json` (looking for at least 4 enchanted golden apples in a ruined portal chest):

```json
[
    {
        "item": "minecraft:enchanted_golden_apple",
        "range": {
            "min": 4,
            "max": 10000
        },
        "slot": 0
    }
]
```

- Use lootinator CLI like this (assuming the constraints and loot table files are in the same directory as the executable):
```bash
./lootinator-cli --loot-table ruined_portal.json --constraint-file constraints_4_notches.json -o four_notches.cu -v 26.1
```

- Compile the CUDA source code file (four_notches.cu). This requires `nvcc` to be available on the host machine.
```bash
nvcc four_notches.cu -o four_notches
```

- Run the compiled code, saving results to the output file `out.txt`. Requires a CUDA-capable GPU.
```bash
./four_notches 1> out.txt
```

## Features

### CLI
- Support for Minecraft 1.13 and above (experimental).
- Support for any loot table defined as a json file (experimental).
- Two modes of operation - *seedcracking* (finding the exact specified chest contents, enable with the `-sc` flag) and *seedfinding* (default, finding a chest with at least the specified items, and potentially others)
- Built-in automated benchmarking (turn off with the `-sk` flag)

### API
- supports all the operations provided by the CLI, see `lootinator/include/lootinator.h` for available functions