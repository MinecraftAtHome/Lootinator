#ifndef LOOTINATOR_KGEN_CUDA_SOURCE_GEN_H
#define LOOTINATOR_KGEN_CUDA_SOURCE_GEN_H

#include "lootinator/kgen/kernel.hpp"

namespace kgen {
	typedef std::vector<ConfiguredKernel> KernelPipeline;
	
	void generate_runner_source(kgen::KernelPipeline& kernel_config, std::ostream& fout);
}

#endif // LOOTINATOR_KGEN_CUDA_SOURCE_GEN_H
