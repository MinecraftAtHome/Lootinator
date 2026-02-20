#include <fstream>
#include <iostream>
#include <vector>

#include "lootinator/lootinator.h"
#include "lootinator/utility/debug.h"

#include "lootinator/data/loot_data.hpp"
#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "lootinator/kgen/secondary_bruteforce_kernel.hpp"
#include "lootinator/kgen/cuda_source_gen.hpp"

int main() {
	kgen::KernelGenConfig kgen_config = {false,
		"../../example/src/simple_constraints_2.json",
		"../../example/src/item_map.txt",
		"../../example/src/ruined_portal.json",
		mc::MC_1_21_TO_1_21_9};

	std::vector<kgen::ConfiguredKernel> kernels1;
	kgen::BruteforceKernel::gen_kernels(kernels1, kgen_config);

	std::vector<kgen::ConfiguredKernel> kernels2;
	kgen::SecondaryBruteforceKernel::gen_kernels(kernels2, kgen_config);

	kgen::KernelPipeline pipeline({{kernels1[0], kernels2[0]}});
	std::ofstream fout("ruined_portal.cu");
	kgen::generate_runner_source(pipeline, fout);
	fout.close();
}