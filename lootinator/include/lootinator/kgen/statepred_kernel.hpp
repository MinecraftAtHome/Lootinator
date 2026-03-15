#ifndef LOOTINATOR_KERNGEN_STATEPRED_KERNEL_H
#define LOOTINATOR_KERNGEN_STATEPRED_KERNEL_H

#include "lootinator/kgen/bruteforce_kernel.hpp"

namespace kgen {
	class StatepredKernel : public BruteforceKernel {
	  public:
		static void gen_kernels(
			std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config);

		loot::Constraint& target_constraint;
		data::LootEntry* entry;
		uint32_t prediction_bound;

		StatepredKernel(data::LootTableRoot& root_node, loot::Constraint& target_constraint, data::LootEntry* entry, const kgen::KernelGenConfig& kgen_config);

	  protected:
		virtual ConfiguredKernel generate() override;

		std::string to_string();

		virtual void generate_forward_filter(std::ostream& out)
	};
}

#endif
