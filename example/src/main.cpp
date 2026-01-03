#include <vector>
#include <fstream>
#include <iostream>

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
	std::vector<loot::Constraint> constr = loot::parse_constraints_from_json("../../example/src/example_constraints.json");
    std::ifstream f("../../example/src/ruined_portal.json");
	nlohmann::json loot_table_json = nlohmann::json::parse(f);

	data::LootTableRoot root = data::LootTableRoot(loot_table_json, "../../example/src/item_map.txt", mc::VersionRange::MC_1_16_TO_1_20);
	root.add_constraints(constr);
	
	//std::cout << root.children[1]->get_min_lcg_advancement() << '\n' << root.children[1]->get_max_lcg_advancement() << '\n';
	
	root.print(0);
}