#ifndef LOOTINATOR_KERNGEN_STATEPRED_KERNEL_H
#define LOOTINATOR_KERNGEN_STATEPRED_KERNEL_H

#include "lootinator/kgen/bruteforce_kernel.hpp"

namespace kgen {
	class StatepredKernel : public BruteforceKernel {
	  public:
		static void gen_kernels(
			std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config);

		StatepredKernel(data::LootTableRoot& root_node, loot::Constraint& target_constraint, const kgen::KernelGenConfig& kgen_config);

	  protected:
		virtual ConfiguredKernel generate() override;

		std::string to_string();

		virtual void emit_cuda_for_pool(std::ostream& out, data::LootPool* pool, int pool_idx);
	};
}

#endif
