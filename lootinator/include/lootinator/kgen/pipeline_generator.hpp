#ifndef LOOTINATOR_KGEN_PIPELINE_GENERATOR_H
#define LOOTINATOR_KGEN_PIPELINE_GENERATOR_H

#include "lootinator/kgen/cuda_source_gen.hpp"

namespace kgen {
	class PipelineGenerator {
		const KernelGenConfig& config;
		std::vector<KernelPipeline> pipelines;

	public:
		PipelineGenerator(const KernelGenConfig& kgen_config);

		PipelineGenerator& add_bruteforce();
		//PipelineGenerator& add_state_prediction();
		//...

		const std::vector<KernelPipeline>& build() const;

	private:
		bool data_has_enchantments();
	};
};

#endif