#include <fstream>
#include <iostream>
#include <vector>

#include "lootinator/lootinator.h"
#include "lootinator/utility/debug.h"

#include "lootinator/kgen/pipeline_generator.hpp"

int main() {
	kgen::KernelGenConfig kgen_config = kgen::KernelGenConfig(mc::MC_1_14_TO_1_15,
#ifdef _WIN32
		"../../../../example/src/chests_jungle_temple.json",
		"../../../../example/src/midas.json",
		"../../../../example/src/item_map_jt.txt",
#elif __linux__
		"../../example/src/chests_jungle_temple.json",
		"../../example/src/midas.json",
		"../../example/src/item_map_jt.txt",
#endif
		true);

	kgen::PipelineGenerator pipeline_gen(kgen_config);
	const auto& pipelines = pipeline_gen.add_bruteforce().build();

	for (int k = 0; k < pipelines.size(); k++) {
		std::ofstream fout("ruined_portal_" + std::to_string(k) + ".cu");
		kgen::generate_runner_source(pipelines[k], fout);
		fout.close();
	}
}