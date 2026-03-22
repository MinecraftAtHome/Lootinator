#ifndef LOOTINATOR_KGEN_CUDA_SOURCE_GEN_H
#define LOOTINATOR_KGEN_CUDA_SOURCE_GEN_H

#include "lootinator/kgen/kernel.hpp"

namespace kgen {
	typedef std::vector<ConfiguredKernel> KernelPipeline;
	
	void generate_runner_source(const kgen::KernelPipeline& kernel_config, std::ostream& fout);
	int generate_benchmarker_source(const std::vector<KernelPipeline>& kernel_configs, std::ostream& fout);
}

#endif // LOOTINATOR_KGEN_CUDA_SOURCE_GEN_H
