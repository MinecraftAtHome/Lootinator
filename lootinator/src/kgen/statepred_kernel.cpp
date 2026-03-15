#include "lootinator/kgen/statepred_kernel.hpp"
#include "lootinator/global_settings.hpp"

#include <iostream>

namespace kgen {
	static void traverse_and_build(std::vector<ConfiguredKernel>& out, data::LootTableRoot& root,
		const kgen::KernelGenConfig& kgen_config, data::LootTreeNode* node) {
		for (auto child : node->children) {
			traverse_and_build(out, root, kgen_config, child);
		}

		CAST_CHILD(entry, data::LootEntry, node);
		if (entry == nullptr) {
			return;
		}

		for (auto& constraint : node->constraints) {
			StatepredKernel spk(root, entry, constraint, kgen_config);
			out.push_back(spk.generate());
		}
	}

	void kgen::StatepredKernel::gen_kernels(
		std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config) {
		data::LootTableRoot root = data::LootTableRoot(
			kgen_config.loot_table_json, kgen_config.item_map, kgen_config.version);

		std::vector<loot::Constraint> merged_constraints;

		std::function<bool(const loot::Constraint& a, const loot::Constraint& b)> cmp_func =
			[](const loot::Constraint& a, const loot::Constraint& b) { return a.item == b.item; };
		merge_contraints(kgen_config.constraints, merged_constraints, cmp_func);

		try {
			root.add_constraints(merged_constraints);
		} catch (std::exception& ex) {
			std::cout << ex.what() << "\n";
		}

		traverse_and_build(out, root, kgen_config, &root);
	}

	StatepredKernel::StatepredKernel(data::LootTableRoot& root_node, data::LootEntry* entry,
		loot::Constraint& target_constraint, const kgen::KernelGenConfig& kgen_config)
		: BruteforceKernel(root_node, kgen_config), target_constraint(target_constraint) {
	
		this->entry = entry;

		// compute the nextInt bound we're predicting on - here it's just the total weight of the loot table
		// but int the future (TODO) we might want to predict on enchantment id, roll count, durability...
		CAST_CHILD(pool, data::LootPool, entry->parent);
		this->prediction_bound = pool->get_total_weight();
	}

	ConfiguredKernel kgen::StatepredKernel::generate() {
		uint32_t threads_per_batch = (UINT64_C(1) << 32);
		uint32_t total_threads = (UINT64_C(1) << 48) / this->prediction_bound;
		uint32_t end_batch = total_threads / threads_per_batch;
		return ConfiguredKernel{this->name,
			to_string(),
			total_threads,
			threads_per_batch,
			global_settings.THREADS_PER_BLOCK,
			combined_shared_memory,
			0,
			0,
			end_batch,
			UINT32_C(1) << 19};
	}

	std::string kgen::StatepredKernel::to_string() {
		return std::string();
	}

	// ------------------------------------------------

	

	void StatepredKernel::emit_function_set_count(std::ostream& out, const SharedEntryData& data) {
		out << "i32 item_count = nextIntBounded(&loot_seed, " << data.min_count << ", " << data.max_count << ");";
	}

	void StatepredKernel::emit_entry_function_enchant_randomly(std::ostream& out, const SharedEntryData& data) {
		int bpe = this->kgen_config.bytes_per_entry;

		if (!(data.enchantment_mask & 1)) {
			
			out << R"(
			u32 enchantment_count = entry_data & 0xff; // [8b]
			i32 enchant_id = enchantment_count != 0 ? nextInt(&loot_seed, enchantment_count) : 64;

			bool r = ((enchantment_mask >> 1) & (1 << enchant_id));
			u64 m = !r - 1;
			loot_seed = (loot_seed * (1|(25214903917&m)) + (11&m)) & MASK_48;)";
		}
	}

	void StatepredKernel::emit_entry_function_enchant_with_levels(std::ostream& out, const SharedEntryData& data) {
		out << "if (enchantment_mask & 1) { enchant_with_levels_function(&loot_seed, "
			   "&(data[enchantment_mask >> 1])); }";
	}

	void StatepredKernel::emit_entry_function_apply_damage(std::ostream& out, const SharedEntryData& data) {
		std::string apply_damage_bitmask = create_apply_damage_item_mask(pool);
		std::string one = pool->children.size() > 32 ? "((u64)1)" : "((u32)1)";


		out << "if (" << apply_damage_bitmask << " & (" << one << " << " << item_idx << R"()) {
			loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
		})";
	}

	void StatepredKernel::emit_state_prediction_entry_handler(std::ostream& out) {
		for (auto& child : entry->children) {
			CAST_CHILD(func, data::LootFunctionData, child); // no nullptr

			switch (func->type) {
				case data::ENCHANT_WITH_LEVELS: {
					emit_entry_function_enchant_with_levels(out);
					break;
				}
				case data::ENCHANT_RANDOMLY: {
					emit_entry_function_enchant_randomly(out);
					break;
				}
				case data::APPLY_DAMAGE: {
					emit_entry_function_apply_damage(out, pool);
					break;
				}
				case data::SET_COUNT: {
					emit_entry_function_set_count(out);
					break;
				}
			}
		}
	}

	void StatepredKernel::generate_statepred_filter(std::ostream& out) {
		out << "__device__ void statepred_filter(u64 state, u32 data[], u64* result_array, u32* result_count) {\n";
		out << "i32 calculated_count = 0; // don't know if it will satisfy constraint requirements\n";
		emit_state_prediction_entry_handler(out); // initializes calculated_count, processes entry functions

		CAST_CHILD(pool, data::LootPool, entry->parent);
		int pool_off = this->pool_memory_offsets[pool->child_index];
		materialize_level(pool);
		std::string item_ident = std::to_string(target_constraint.item);
		std::string arrayPlusIndex = var_name_map[item_ident];

		out << "i32 local_constraints[" << (pool->constraints.size() + 1) << "] = {0};\n";
		out << arrayPlusIndex << " = calculated_count;\n";
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

		out << "\n}"; // end of function
	}
}


