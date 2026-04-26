#include <fstream>
#include <iostream>
#include <sstream>

#include "lootinator/assertions.h"
#include "lootinator/mc/minecraft.hpp"
#include "lootinator/probability/loot_prob.h"

int LOOTINATOR_EXTERN tests_test_kgen_config(int argc, char** const argv) {
	(void)argc;
	(void)argv;

	std::string simple_constraints = R"(
        [
            {
                "item": "minecraft:obsidian",
                "range": {
                    "min": 7,
                    "max": 100
                },
                "slot": 0
            }
        ]
    )";

	std::string simple_constraints2 = R"(
        [
            {
                "item": "minecraft:gold_block",
                "range": {
                    "min": 2,
                    "max": 100
                },
                "slot": 0
            }
        ]
    )";

	std::string constraints = R"(
        [
            {
                "item": "minecraft:golden_sword",
                "range": {
                    "min": 1,
                    "max": 1
                },
                "slot": 0,
                "attributes": [
                    {"type": "looting", "level": 3}
                ]
            },
            {
                "item": "minecraft:golden_carrot",
                "range": {
                    "min": 10,
                    "max": 100
                },
                "slot": 0
            },
            {
                "item": "minecraft:gold_block",
                "range": {
                    "min": 4,
                    "max": 100
                },
                "slot": 0
            },
            {
                "item": "minecraft:flint_and_steel",
                "range": {
                    "min": 1,
                    "max": 1
                },
                "slot": 0
            },
            {
                "item": "minecraft:golden_axe",
                "range": {
                    "min": 1,
                    "max": 1
                },
                "slot": 0,
                "attributes": [
                    {"type": "efficiency"}
                ]
            }
        ]
    )";

	std::string loot_table = R"(
        {
        "type": "minecraft:chest",
        "pools": [
            {
            "bonus_rolls": 0.0,
            "entries": [
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "add": false,
                    "count": {
                        "type": "minecraft:uniform",
                        "max": 2.0,
                        "min": 1.0
                    },
                    "function": "minecraft:set_count"
                    }
                ],
                "name": "minecraft:obsidian",
                "weight": 40
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "add": false,
                    "count": {
                        "type": "minecraft:uniform",
                        "max": 4.0,
                        "min": 1.0
                    },
                    "function": "minecraft:set_count"
                    }
                ],
                "name": "minecraft:flint",
                "weight": 40
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "add": false,
                    "count": {
                        "type": "minecraft:uniform",
                        "max": 18.0,
                        "min": 9.0
                    },
                    "function": "minecraft:set_count"
                    }
                ],
                "name": "minecraft:iron_nugget",
                "weight": 40
                },
                {
                "type": "minecraft:item",
                "name": "minecraft:flint_and_steel",
                "weight": 40
                },
                {
                "type": "minecraft:item",
                "name": "minecraft:fire_charge",
                "weight": 40
                },
                {
                "type": "minecraft:item",
                "name": "minecraft:golden_apple",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "add": false,
                    "count": {
                        "type": "minecraft:uniform",
                        "max": 24.0,
                        "min": 4.0
                    },
                    "function": "minecraft:set_count"
                    }
                ],
                "name": "minecraft:gold_nugget",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "function": "minecraft:enchant_randomly",
                    "options": "#minecraft:on_random_loot"
                    }
                ],
                "name": "minecraft:golden_sword",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "function": "minecraft:enchant_randomly",
                    "options": "#minecraft:on_random_loot"
                    }
                ],
                "name": "minecraft:golden_axe",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "function": "minecraft:enchant_randomly",
                    "options": "#minecraft:on_random_loot"
                    }
                ],
                "name": "minecraft:golden_hoe",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "function": "minecraft:enchant_randomly",
                    "options": "#minecraft:on_random_loot"
                    }
                ],
                "name": "minecraft:golden_shovel",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "function": "minecraft:enchant_randomly",
                    "options": "#minecraft:on_random_loot"
                    }
                ],
                "name": "minecraft:golden_pickaxe",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "function": "minecraft:enchant_randomly",
                    "options": "#minecraft:on_random_loot"
                    }
                ],
                "name": "minecraft:golden_boots",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "function": "minecraft:enchant_randomly",
                    "options": "#minecraft:on_random_loot"
                    }
                ],
                "name": "minecraft:golden_chestplate",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "function": "minecraft:enchant_randomly",
                    "options": "#minecraft:on_random_loot"
                    }
                ],
                "name": "minecraft:golden_helmet",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "function": "minecraft:enchant_randomly",
                    "options": "#minecraft:on_random_loot"
                    }
                ],
                "name": "minecraft:golden_leggings",
                "weight": 15
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "add": false,
                    "count": {
                        "type": "minecraft:uniform",
                        "max": 12.0,
                        "min": 4.0
                    },
                    "function": "minecraft:set_count"
                    }
                ],
                "name": "minecraft:glistering_melon_slice",
                "weight": 5
                },
                {
                "type": "minecraft:item",
                "name": "minecraft:golden_horse_armor",
                "weight": 5
                },
                {
                "type": "minecraft:item",
                "name": "minecraft:light_weighted_pressure_plate",
                "weight": 5
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "add": false,
                    "count": {
                        "type": "minecraft:uniform",
                        "max": 12.0,
                        "min": 4.0
                    },
                    "function": "minecraft:set_count"
                    }
                ],
                "name": "minecraft:golden_carrot",
                "weight": 5
                },
                {
                "type": "minecraft:item",
                "name": "minecraft:clock",
                "weight": 5
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "add": false,
                    "count": {
                        "type": "minecraft:uniform",
                        "max": 8.0,
                        "min": 2.0
                    },
                    "function": "minecraft:set_count"
                    }
                ],
                "name": "minecraft:gold_ingot",
                "weight": 5
                },
                {
                "type": "minecraft:item",
                "name": "minecraft:bell"
                },
                {
                "type": "minecraft:item",
                "name": "minecraft:enchanted_golden_apple"
                },
                {
                "type": "minecraft:item",
                "functions": [
                    {
                    "add": false,
                    "count": {
                        "type": "minecraft:uniform",
                        "max": 2.0,
                        "min": 1.0
                    },
                    "function": "minecraft:set_count"
                    }
                ],
                "name": "minecraft:gold_block"
                }
            ],
            "rolls": {
                "type": "minecraft:uniform",
                "max": 8.0,
                "min": 4.0
            }
            }
        ],
        "random_sequence": "minecraft:chests/ruined_portal"
    }
    )";

#define EPS 1e-6
#define ASSERT_FLOAT_EQ(a, b) ASSERT_LE(fabs((a) - (b)), EPS)

	double config_probability;

	kgen::KernelGenConfig kgen_config =
		kgen::KernelGenConfig(mc::VersionRange::MC_1_21_TO_1_21_10, loot_table, constraints, false, "");
	config_probability = prob::get_probability_of_config(kgen_config);
	ASSERT_FLOAT_EQ(config_probability, 1.77305e-09);

	kgen_config = kgen::KernelGenConfig(
		mc::VersionRange::MC_1_21_TO_1_21_10, loot_table, simple_constraints, true, "");
	config_probability = prob::get_probability_of_config(kgen_config);
	ASSERT_FLOAT_EQ(config_probability, 0.000680201);

	kgen_config = kgen::KernelGenConfig(
		mc::VersionRange::MC_1_21_TO_1_21_10, loot_table, simple_constraints2, false, "");
	config_probability = prob::get_probability_of_config(kgen_config);
	ASSERT_FLOAT_EQ(config_probability, 0.007537);

#undef EPS
#undef ASSERT_FLOAT_EQ

	return 0;
}