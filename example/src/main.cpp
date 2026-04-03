#include "lootinator/lootinator.h"
#include <fstream>
#include <iostream>

int main() {
	std::string loot_table_file =
#ifdef _WIN32
		"../../../../example/src/ruined_portal.json";
#elif __linux__
		"../../example/src/buried_treasure.json";
#endif

	std::string constraints_file =
#ifdef _WIN32
		"../../../../example/src/speedrun_starter_pack.json";
#elif __linux__
		"../../example/src/ask_for_help.json";
#endif

	std::ofstream fout("bench_full.cu");
	std::string s;

	// LootinatorError generate_best_pipeline_heur(
	// 	const std::string loot_table_filepath,
	// 	const std::string constraint_filepath,
	// 	const mc::VersionRange version_range,
	// 	const bool use_seedcracking_mode,
	// 	std::string* result
	// );

	loot::LootinatorError err_code = loot::generate_best_pipeline_heur(
		loot_table_file, constraints_file, mc::MC_1_21_TO_1_21_10, true, &s);
	if (err_code == loot::LootinatorError::SUCCESS) {
		fout << s;
		std::cout << "Cuda generation was successful.\n";
	} else {
		std::cout << "Lootinator failed: " << loot::parse_errno(err_code) << '\n';
	}
}