# Lootinator CLI Usage Guide

## Quick Start

Lootinator does not find loot seeds directly, it instead generates cuda kernels which can in turn be compiled and run on any
NVIDIA graphics card.

- In order to run the cli, you will need the json of the loot table you are targetting. These can be found for all versions here: https://mcasset.cloud/26.1.2/data/minecraft/loot_table/chests

- In addition, you will need a constraint file describing all the constraints you are targetting. The json schematic is as follows:
```json
[
    {
        "item": <item_name>, // the name of the item you are targetting, must be prefixed with minecraft:
        "range": { // the minimum and maximum item count for this constraint
            "min": <item_min>,
            "max": <item_max>
        },
        "slot": slot_index>, // the slot of the item between 0-27 (currently ignored)
		"attributes": [ // optional attribute data for enchantments
			{"type": <enchant_name>, "level": <enchant_level>} // level is optional. 
		]
    }
]
```

- With both of these json files, we can call the lootinator-cli

```bash
./lootinator-cli --loot-table <file_path.json> --constraint-file <file_path.json> -o
                    <filepath> [-sc] [-sk] [-v <version>]
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