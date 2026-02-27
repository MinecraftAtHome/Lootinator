#ifndef LOOTINATOR_KERNGEN_BRUTEFORCE_KERNEL_H
#define LOOTINATOR_KERNGEN_BRUTEFORCE_KERNEL_H

#include "lootinator/kgen/kernel.hpp"

namespace kgen {
	class BruteforceKernel : public Kernel {
	  public:
		static void gen_kernels(
			std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config);
		BruteforceKernel(data::LootTableRoot& root_node, kgen::KernelGenConfig kgen_config);

	  protected:
		virtual ConfiguredKernel generate() override;

		// maps constraint signatures (combinations of item type, enchantment, ench level)
		// to array names + indices like this: array[idx]
		std::unordered_map<std::string, std::string> var_name_map;

		std::string to_string();
		std::string create_forbidden_item_mask(data::LootPool* pool);
		//void emit_skip_for_entry(std::ostream& out, data::LootEntry* entry);
		//void emit_cuda_for_entry(std::ostream& out, data::LootEntry* entry);
		void emit_cuda_for_pool(std::ostream& out, data::LootPool* pool, int pool_idx);
		void generate_forward_filter(std::ostream& out);

		void materialize_level(data::LootTreeNode* node);

		void setup_entry_memory(data::LootPool* pool);
		virtual void setup_shared_memory() override;
		virtual void fill_function_shared_mem(data::LootTreeNode* current) override;

		void setup_enchant_with_levels(data::LootTreeNode* node, int total_entry_offset, int entry_idx, const uint32_t offset_before_surgery);
	};
} // namespace kgen

#endif