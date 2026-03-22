#include <fstream>
#include <iostream>
#include <vector>

#include "lootinator/lootinator.h"
#include "lootinator/utility/debug.h"

#include "lootinator/kgen/pipeline_generator.hpp"

int main() {
	kgen::KernelGenConfig kgen_config = kgen::KernelGenConfig(mc::MC_1_21_TO_1_21_10,
#ifdef _WIN32
		"../../../../example/src/ruined_portal.json",
		"../../../../example/src/speedrun_starter_pack.json",
		"../../../../example/src/item_map.txt",
#elif __linux__
		"../../example/src/ruined_portal.json",
		"../../example/src/speedrun_starter_pack.json",
		"../../example/src/item_map.txt",
#endif
		true);

	kgen::PipelineGenerator pipeline_gen(kgen_config);
	const auto& pipelines = pipeline_gen.add_state_prediction().add_bruteforce().build();

	std::ofstream fout("full_bench.cu");
	kgen::generate_benchmarker_source(pipelines, fout);

	//for (int k = 0; k < pipelines.size(); k++) {
	//	std::ofstream fout("midas" + std::to_string(k) + ".cu");
	//	kgen::generate_runner_source(pipelines[k], fout);
	//	fout.close();
	//}
}