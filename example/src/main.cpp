#include <vector>
#include <fstream>

#include "lootinator/lootinator.h"
#include "lootinator/utility/debug.h"

#include "lootinator/data/loot_data.hpp"

/*
loot table: ruined portal
constraints:
[
	{
		"item": 4,
		"range": {
            "min": 20,
            "max": 100000
        }
	}
]

pool 0  {
	lcg-advance 1;
	roll 0 {
		//...
		case 4 {
			pool-assert >= 20;
		}
		//...
	}
}
*/

int main() {
	//std::vector<loot::Constraint> constr = loot::parse_constraints_from_json("../../example/src/example_constraints.json");
    std::ifstream f("../../lootinator/tests/data/end_city_1_21_8.json");
	nlohmann::json loot_table_json = nlohmann::json::parse(f);

	data::LootTableRoot root = data::LootTableRoot(loot_table_json, "../../lootinator/tests/data/item_map_end_city.txt", mc::VersionRange::MC_1_21_TO_1_21_9);
	//root.add_constraints(constr);
	
	root.print(0);
}