#include "lootinator/lootinator.h"
#include <fstream>
#include <iostream>


int main() {
	std::string loot_table_file = 
		#ifdef _WIN32
				"../../../../example/src/ruined_portal.json";
		#elif __linux__
				"../../example/src/ruined_portal.json";
		#endif

	std::string constraints_file = 
		#ifdef _WIN32
				"../../../../example/src/speedrun_starter_pack.json";
		#elif __linux__
				"../../example/src/speedrun_starter_pack.json";
		#endif

	std::ofstream fout("bench_full.cu");
	std::string s;

	loot::LootinatorError err_code = loot::generate_benchmark_source(
		loot_table_file,
		constraints_file,
		mc::MC_1_21_TO_1_21_10,
		true,
		&s
	);
	if (err_code == loot::LootinatorError::SUCCESS) {
		fout << s;
		std::cout << "Cuda generation was successful.\n";
	}
	else {
		std::cout << "Lootinator failed: " << loot::parse_errno(err_code) << '\n';
	}
}