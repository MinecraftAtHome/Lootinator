#include "lootinator/kgen/pipeline_generator.hpp"

#include <fstream>

// all the kernels
#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "lootinator/kgen/secondary_bruteforce_kernel.hpp"
#include "lootinator/kgen/statepred_kernel.hpp"
// ...

namespace kgen {
	PipelineGenerator::PipelineGenerator(
		const KernelGenConfig& kgen_config) // TODO add number of kernels returned
		: config(kgen_config) {
	}

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

	PipelineGenerator& kgen::PipelineGenerator::add_state_prediction() {
		std::vector<ConfiguredKernel> kernels1;
		StatepredKernel::gen_kernels(kernels1, config);

		std::vector<kgen::ConfiguredKernel> kernels2;
		if (data_has_enchantments()) {
			kgen::SecondaryBruteforceKernel::gen_kernels(kernels2, config);
		}

		for (auto& kernel1 : kernels1) {
			printf("%f\n", kernel1.heuristic);
			KernelPipeline statepred_pipeline;
			statepred_pipeline.push_back(kernel1);
			if (!kernels2.empty()) {
				statepred_pipeline.push_back(kernels2[0]);
			}
			pipelines.push_back(statepred_pipeline);
		}

		return *this;
	}

	std::vector<KernelPipeline> PipelineGenerator::build() {
		// TODO calc heuristic performance for each pipeline,
		// sort, return top-scoring N
		std::sort(pipelines.begin(),
			pipelines.end(),
			[](const kgen::KernelPipeline& a, const kgen::KernelPipeline& b) {
				return a[0].heuristic < b[0].heuristic;
			});
		return pipelines;
	}
} // namespace kgen
