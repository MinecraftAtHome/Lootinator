#ifndef LOOTINATOR_KGEN_CUDA_SOURCE_GEN_H
#define LOOTINATOR_KGEN_CUDA_SOURCE_GEN_H

#include "lootinator/kgen/kernel.hpp"

namespace kgen {
	int generate_benchmarker_source(
		std::vector<kgen::ConfiguredKernel> kernel_configs, std::ostream& fout);
	void generate_runner_source(kgen::ConfiguredKernel& kernel_config, std::ostream& fout);
} // namespace kgen

#endif // LOOTINATOR_KGEN_CUDA_SOURCE_GEN_H
