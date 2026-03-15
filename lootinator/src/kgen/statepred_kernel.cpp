#include "lootinator/kgen/statepred_kernel.hpp"


namespace kgen {

	void kgen::StatepredKernel::gen_kernels(std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config) {
	}

	kgen::StatepredKernel::StatepredKernel(data::LootTableRoot& root_node, loot::Constraint& target_constraint, const kgen::KernelGenConfig& kgen_config) 
		: BruteforceKernel(root_node, kgen_config) {
	}

	ConfiguredKernel kgen::StatepredKernel::generate() {
		return ConfiguredKernel();
	}

	std::string kgen::StatepredKernel::to_string() {
		return std::string();
	}

	void kgen::StatepredKernel::emit_cuda_for_pool(std::ostream& out, data::LootPool* pool, int pool_idx) {
	}
}


