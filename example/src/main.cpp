#include <vector>
#include <fstream>
#include <iostream>

#include "lootinator/lootinator.h"
#include "lootinator/utility/debug.h"

#include "lootinator/data/loot_data.hpp"
#include "lootinator/kgen/bruteforce_kernel.hpp"

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
	std::vector<loot::Constraint> constr = loot::parse_constraints_from_json("../../example/src/simple_constraints_2.json");
    std::ifstream f("../../example/src/ruined_portal.json");
	nlohmann::json loot_table_json = nlohmann::json::parse(f);

	data::LootTableRoot root = data::LootTableRoot(loot_table_json, "../../example/src/item_map.txt", mc::VersionRange::MC_1_21_TO_1_21_9);

	try {
		root.add_constraints(constr);
	}		
    catch (std::exception &ex) {
		std::cout << ex.what() << "\n";
	}

	std::vector<kgen::ConfiguredKernel> kernels;
	kgen::BruteforceKernel::gen_kernels(root, kernels);

	std::ofstream fout("first_cuda.cu");
	fout << kernels[0].code << '\n';
	fout.close();

	//kgen::StatePredictionKernel::gen_kernels(root, kernels);
	//...

	//root.print(0);
}