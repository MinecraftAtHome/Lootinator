#include <fstream>
#include <iostream>
#include <vector>

#include "lootinator/lootinator.h"
#include "lootinator/utility/debug.h"

#include "lootinator/data/loot_data.hpp"
#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "lootinator/kgen/cuda_source_gen.hpp"

int main() {
	kgen::KernelGenConfig kgen_config = {false,
		"../../example/src/simple_constraints_2.json",
		"../../example/src/item_map.txt",
		"../../example/src/ruined_portal.json",
		mc::MC_1_21_TO_1_21_9};
	std::vector<kgen::ConfiguredKernel> kernels;

	kgen::BruteforceKernel::gen_kernels(kernels, kgen_config);

	std::ofstream fout("ruined_portal.cu");
	generate_runner_source(kernels[0], fout);
	fout.close();
}