#include "lootinator/kgen/pipeline_generator.hpp"

#include <fstream>

// all the kernels
#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "lootinator/kgen/secondary_bruteforce_kernel.hpp"
// ...


namespace kgen {
	PipelineGenerator::PipelineGenerator(const KernelGenConfig& kgen_config) // TODO add number of kernels returned
		: config(kgen_config) {}

	// FIXME this treats every attribute as enchantment
	bool PipelineGenerator::data_has_enchantments() {
		for (auto& constr : config.constraints) {
			if (!constr.attributes.empty()) {
				return true;
			}
		}
		return false;
	}

	PipelineGenerator& kgen::PipelineGenerator::add_bruteforce() {
		KernelPipeline bruteforce_pipeline;

		std::vector<kgen::ConfiguredKernel> kernels1;
		kgen::BruteforceKernel::gen_kernels(kernels1, config);
		bruteforce_pipeline.push_back(kernels1[0]);

		if (data_has_enchantments()) {
			std::vector<kgen::ConfiguredKernel> kernels2;
			kgen::SecondaryBruteforceKernel::gen_kernels(kernels2, config);
			bruteforce_pipeline.push_back(kernels2[0]);
		}

		pipelines.push_back(bruteforce_pipeline);
		return *this;
	}

	//PipelineGenerator& kgen::PipelineGenerator::add_state_prediction() {
	//	// TODO create the pipelines
	//
	//	return *this;
	//}

	const std::vector<KernelPipeline>& PipelineGenerator::build() const {
		// TODO calc heuristic performance for each pipeline, 
		// sort, return top-scoring N

		return pipelines;
	}
}
