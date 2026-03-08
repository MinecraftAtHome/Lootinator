#include <fstream>
#include <iostream>
#include <vector>

#include "lootinator/lootinator.h"
#include "lootinator/utility/debug.h"

#include "lootinator/kgen/pipeline_generator.hpp"

int main() {
	kgen::KernelGenConfig kgen_config = kgen::KernelGenConfig(mc::MC_1_21_TO_1_21_9,
#ifdef _WIN32
		"../../../../example/src/chests_ancient_city.json",
		"../../../../example/src/midas.json",
		"../../../../example/src/item_map_ac.txt",
#elif __linux__
		"../../../../example/src/chests_ancient_city.json",
		"../../../../example/src/midas.json",
		"../../../../example/src/item_map_ac.txt",
#endif
		true);

	kgen::PipelineGenerator pipeline_gen(kgen_config);
	const auto& pipelines = pipeline_gen.add_bruteforce().build();

	for (int k = 0; k < pipelines.size(); k++) {
		std::ofstream fout("ac_" + std::to_string(k) + ".cu");
		kgen::generate_runner_source(pipelines[k], fout);
		fout.close();
	}
}