#include "lootinator/kgen/statepred_kernel.hpp"


namespace kgen {

	void kgen::StatepredKernel::gen_kernels(std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config) {
	}

	StatepredKernel::StatepredKernel(data::LootTableRoot& root_node, loot::Constraint& target_constraint, 
		data::LootEntry* entry, const kgen::KernelGenConfig& kgen_config)
		: BruteforceKernel(root_node, kgen_config), target_constraint(target_constraint) {
	
		this->entry = entry;

		// compute the nextInt bound we're predicting on - here it's just the total weight of the loot table
		// but int the future (TODO) we might want to predict on enchantment id, roll count, durability...
		CAST_CHILD(pool, data::LootPool, entry->parent);
		this->prediction_bound = pool->get_total_weight();
	}

	ConfiguredKernel kgen::StatepredKernel::generate() {
		return ConfiguredKernel();
	}

	std::string kgen::StatepredKernel::to_string() {
		return std::string();
	}

	void StatepredKernel::generate_forward_filter(std::ostream& out) {
		out << "__device__ void statepred_filter(u64 thread_idx, u32 data[], u64* result_array, u32* result_count) {\n";
		
		out << "uint64_t state = predicted_state;\n";

		CAST_CHILD(pool, data::LootPool, entry->parent);
		int pool_off = this->pool_memory_offsets[pool->child_index];

		out << "{\n"; // start of pool handler block

		int first_item_count = 1;// TODO FIXME
		if (!pool->constraints.empty()) {
			out << "i32 local_constraints[2] = {0, " << first_item_count << "};\n";
		}
		out << "for (i32 roll = 0; roll < " << (pool->rolls.max-1) << "; roll++) {\n";

		extract_data_prefix(out, pool, pool_off);

		for (auto& func : this->kgen_config.function_order) {
			switch (func) {
				case data::ENCHANT_WITH_LEVELS: {
					emit_function_enchant_with_levels(out);
					break;
				}
				case data::ENCHANT_RANDOMLY: {
					emit_function_enchant_randomly(out);
					break;
				}
				case data::APPLY_DAMAGE: {
					emit_function_apply_damage(out, pool);
					break;
				}
				case data::SET_COUNT: {
					emit_function_set_count(out);
					break;
				}
			}
		}

		out << "\nlocal_constraints[counter_idx] += item_count;\n";
		out << "}\n"; // end of for loop

		// check constraint satisfaction
		for (auto& constraint : pool->constraints) {
			std::string comp = constraint.count_range.min == constraint.count_range.max ? " == " : ">=";
			out << "if (!(local_constraints[1] " << comp << constraint.count_range.min << ")) return false;\n";
		}

		out << "}\n"; // end of pool handler block

		out << "\n}"; // end of function
	}
}


