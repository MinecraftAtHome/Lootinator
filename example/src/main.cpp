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

/*
BEFORE:
Constraints: [
	Constraint{item=23, count_range=RangeInclusive{min=0, max=0}, slot_id=0, attributes=[]}, 
	Constraint{item=23, count_range=RangeInclusive{min=2, max=2}, slot_id=0, attributes=[ItemAttribute{type=21, level=3}]}, 
	Constraint{item=23, count_range=RangeInclusive{min=1, max=100000}, slot_id=0, attributes=[ItemAttribute{type=21, level=-1}]}, ]

AFTER:
Constraints: [
	Constraint{item=23, count_range=RangeInclusive{min=2, max=2}, slot_id=0, attributes=[ItemAttribute{type=21, level=3}]}, 
	Constraint{item=23, count_range=RangeInclusive{min=1, max=100000}, slot_id=0, attributes=[ItemAttribute{type=21, level=-1}]}, 
	Constraint{item=23, count_range=RangeInclusive{min=0, max=0}, slot_id=0, attributes=[]}, 
*/

	try {
		root.add_constraints(constr);
	}		
    catch (std::exception &ex) {
		std::cout << ex.what() << "\n";
	}

	root.print(0);
}