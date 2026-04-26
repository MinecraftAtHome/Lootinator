#include <algorithm>

#include "lootinator/assertions.h"
#include "lootinator/kgen/kernel.hpp"

int LOOTINATOR_EXTERN tests_function_order_dfs(int argc, char** const argv) {
	(void)argc;
	(void)argv;

	try {

		kgen::KernelGenConfig conf(mc::VersionRange::MC_1_21_TO_1_21_10,

			// loot table
			R"(
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
            },
            {
                "function": "minecraft:enchant_randomly"
            }
          ],
          "name": "minecraft:golden_boots",
          "weight": 5
        },
        {
          "type": "minecraft:item",
          "functions": [
            {
                "function": "minecraft:set_damage"
            },
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
                "function": "minecraft:set_damage"
            },
            {
                "function": "minecraft:enchant_randomly"
            }
          ],
          "name": "minecraft:iron_helmet",
          "weight": 40
        }
      ],
      "rolls": {
        "type": "minecraft:uniform",
        "max": 10.0,
        "min": 4.0
      }
    }
  ],
  "random_sequence": "minecraft:chests/ruined_portal"
}
			)",

			// constraints
			R"(
[
	{
		"item": "minecraft:golden_boots",
		"range": {
			"min": 10,
			"max": 10000
		},
		"slot": 0
	}
]
			)",
			false,
      "");

		// there should exist a good ordering here
		ASSERT_EQ(conf.no_fast_filter, false);

		// the only ordering that works
		std::vector<data::LootFunctionType> target_order({data::LootFunctionType::APPLY_DAMAGE,
			data::LootFunctionType::SET_COUNT,
			data::LootFunctionType::ENCHANT_RANDOMLY});

		ASSERT_EQ(target_order.size(), conf.function_order.size());

		bool target_matches_result =
			std::equal(target_order.begin(), target_order.end(), conf.function_order.begin());

		ASSERT_EQ(target_matches_result, true);
	} catch (loot::LootinatorError& err) {
		std::cerr << "Caught LootinatorError: " << err.message << '\n';
		return 1;
	} catch (std::exception& ex) {
		std::cerr << ex.what() << '\n';
		return 1;
	}

	return 0;
}