#include <vector>
#include <fstream>
#include <iostream>

#include "lootinator/lootinator.h"
#include "lootinator/utility/debug.h"

#include "lootinator/data/loot_data.hpp"
#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "lootinator/kgen/cuda_source_gen.hpp"


void copy_test() {
	std::vector<loot::Constraint> constr = loot::parse_constraints_from_json("../../../../example/src/simple_constraints.json");
	std::ifstream f("../../../../example/src/ruined_portal.json");
	nlohmann::json loot_table_json = nlohmann::json::parse(f);

	data::LootTableRoot root = data::LootTableRoot(loot_table_json, "../../../../example/src/item_map.txt", mc::VersionRange::MC_1_21_TO_1_21_9);

	try {
		root.add_constraints(constr);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}


	data::LootTableRoot root2 = root.copy();
	return;
}


int main() {
	//copy_test();
	//return 0;

	std::vector<loot::Constraint> constr = loot::parse_constraints_from_json("../../example/src/simple_constraints.json");
    std::ifstream f("../../example/src/ruined_portal.json");
	nlohmann::json loot_table_json = nlohmann::json::parse(f);
	
	data::LootTableRoot root = data::LootTableRoot(loot_table_json, "../../example/src/item_map.txt", mc::VersionRange::MC_1_21_TO_1_21_9);
	
	try {
		root.add_constraints(constr);
	}		
    catch (std::exception &ex) {
		std::cout << ex.what() << "\n";
	}
	
	kgen::KernelGenConfig kgen_config = {/*seedcracking=*/true};	
	
	std::vector<kgen::ConfiguredKernel> kernels;

	kgen::BruteforceKernel::gen_kernels(root, kernels, kgen_config);

	std::ofstream fout("ruined_portal.cu");
    generate_runner_source(kernels[0], fout);
	fout.close();
}