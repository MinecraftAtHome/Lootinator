#ifndef LOOTINATOR_KERNGEN_STATEPRED_KERNEL_H
#define LOOTINATOR_KERNGEN_STATEPRED_KERNEL_H

#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "lootinator/kgen/secondary_bruteforce_kernel.hpp"

namespace kgen {
	class StatepredKernel : public BruteforceKernel, public SecondaryBruteforceKernel {
	  public:
		static void gen_kernels(
			std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config);

		loot::Constraint& target_constraint;
		data::LootEntry* entry;
		uint32_t prediction_bound;

		StatepredKernel(data::LootTableRoot& root_node, data::LootEntry* target_entry,
			loot::Constraint& target_constraint, const kgen::KernelGenConfig& kgen_config);

		virtual float heuristic() const override;

		struct SharedEntryData {
			uint32_t entry_data;
			uint64_t enchantment_mask;
			uint32_t enchantment_mask_2;
			uint32_t enchantment_count;
			uint32_t item;
			uint32_t item_idx;
			uint32_t min_count;
			uint32_t max_count;

			SharedEntryData(
				int pool_off, int bpe, data::LootEntry* entry, std::vector<uint32_t>& shared_mem) {
				item = entry->next_int_range.min;
				entry_data = shared_mem[pool_off + item * bpe];
				enchantment_mask = shared_mem[pool_off + item * bpe + 1];
				enchantment_mask_2 = shared_mem[pool_off + item * bpe + 2];
				if (bpe >= 3) {
					enchantment_mask |= (static_cast<uint64_t>(enchantment_mask_2) << 32);
				}
				item_idx = entry_data >> 24;
				min_count = (entry_data >> 18) & 0x3f;
				max_count = (entry_data >> 12) & 0x3f;
				enchantment_count = entry_data & 0xff;
			}
		};

		void emit_entry_function_set_count(std::ostream& out, const SharedEntryData& data);
		void emit_entry_function_enchant_randomly(std::ostream& out, const SharedEntryData& data);
		void emit_entry_function_enchant_with_levels(
			std::ostream& out, const SharedEntryData& data);
		void emit_entry_function_apply_damage(
			std::ostream& out, const SharedEntryData& data, data::LootPool* pool);

		virtual ConfiguredKernel generate() override;

	  protected:
		std::string to_string();

		void emit_state_prediction_entry_handler(std::ostream& out, const SharedEntryData& data);

		void generate_statepred_filter_secondary(std::ostream& out);
		virtual void generate_statepred_filter(std::ostream& out);
	};
} // namespace kgen

#endif
