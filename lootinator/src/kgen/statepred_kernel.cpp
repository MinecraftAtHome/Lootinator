#include "lootinator/kgen/statepred_kernel.hpp"
#include "lootinator/global_settings.hpp"
#include "lootinator/utility/mth.h"
#include "lootinator/probability/loot_prob.h"

#include <iostream>
#include <sstream>

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
			for (int pfx = 0; pfx < 2; pfx++) {
				loot::Constraint c = constraint.truncate_attribute(pfx);
				if (c == constraint) {
					continue;
				}

				StatepredKernel spk(root, entry, c, kgen_config);
				out.push_back(spk.generate());
			}

			// we didn't handle the base constraint when truncating
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

		// compute the nextInt bound we're predicting on - here it's just the total weight of the
		// loot table but int the future (TODO) we might want to predict on enchantment id, roll
		// count, durability...
		CAST_CHILD(pool, data::LootPool, entry->parent);
		this->prediction_bound = pool->get_total_weight();

		static int kernel_index = 0;
		kernel_index++;
		name = "statepred_kernel_" + std::to_string(kernel_index);
	}

	ConfiguredKernel kgen::StatepredKernel::generate() {
		uint64_t threads_per_batch = (UINT64_C(1) << 32);
		uint64_t total_threads = (UINT64_C(1) << 48) / this->prediction_bound;
		uint32_t end_batch = total_threads / threads_per_batch;
		return ConfiguredKernel{
			this->name,
			to_string(),
			total_threads,
			threads_per_batch,
			global_settings.THREADS_PER_BLOCK,
			combined_shared_memory,
			0,
			0,
			end_batch,
			UINT32_C(1) << 19,
			this->heuristic(),
		};
	}

	std::string kgen::StatepredKernel::to_string() {
		std::stringstream result;
		Kernel::write_shared_definitions(result);

		BruteforceKernel::generate_forward_filter(result);
		generate_statepred_filter(result);

		data::LootPool* pool = dynamic_cast<data::LootPool*>(this->entry->parent);

		result << c_extern() << R"(__global__ void )" << this->name
			   << "(u64* result_array, u32* result_count, u32* shared_mem_contents,"
				  "u64 offset) {";
		result <<
			R"(
    __shared__ u32 data[)"
			   << combined_shared_memory.size() << R"(];
    if (threadIdx.x < )"
			   << combined_shared_memory.size() << R"() {
        for (int i = threadIdx.x; i < )"
			   << combined_shared_memory.size() << R"(; i += blockDim.x) {
            data[i] = shared_mem_contents[i];
        }
    }
    __syncthreads();

	u64 tid = (u64)blockDim.x * blockIdx.x + threadIdx.x + offset;
	u32 lower17 = tid & ((1ull << 17) - 1);
	u64 upper31 = (tid >> 17) * )"
			   << pool->entry_lookup.size() << R"(;
	
	#pragma unroll
	for (int rem = )"
			   << entry->next_int_range.min << R"(; rem <= )" << entry->next_int_range.max
			   << R"(; rem++) {
		u64 state = ((upper31 + rem) << 17) | lower17;
		statepred_filter(state, data, result_array, result_count);
	}
}
)";
		return result.str();
	}

	// ------------------------------------------------

	void StatepredKernel::emit_entry_function_set_count(
		std::ostream& out, const SharedEntryData& data) {
		out << "calculated_count = nextIntBounded(&loot_seed, " << data.min_count << ", "
			<< data.max_count << ");";
	}

	void StatepredKernel::emit_entry_function_enchant_randomly(
		std::ostream& out, const SharedEntryData& data) {
		if (!(!(data.enchantment_mask & 1) && data.enchantment_count > 0)) {
			return;
		}

		// find enchant randomly
		data::LootFunctionData* func = nullptr;
		for (auto child : entry->children) {
			CAST_CHILD(f, data::LootFunctionData, child);
			if (f->type == data::ENCHANT_RANDOMLY) { // had a nasty = instead of == here
				func = f;
			}
		}
		if (func == nullptr) {
			printf("something went wrong, no enchant_randomly found\n");
			return;
		}

		// based on the constraint, emit proper filters
		out << "i32 enchant_id = " << data.enchantment_count << " != 0 ? nextInt(&loot_seed, "
			<< data.enchantment_count << ") : 64;\n";

		if (target_constraint.attributes.empty()) {
			// no attributes - skip
			out << "bool r = ((" << (data.enchantment_mask >> 1) << R"() & (1ULL << enchant_id));
u64 m = !r - 1;
loot_seed = (loot_seed * (1|(25214903917&m)) + (11&m)) & MASK_48;)";
			return;
		}

		// find index of filtered enchantment in the order vector
		auto& vec = func->enchant_randomly.enchantment_order;
		mc::Enchantment target_ench =
			mc::get_enchantment_from_attribute(target_constraint.attributes[0]);
		int ench_idx = 0;
		for (; ench_idx < vec.size(); ench_idx++) {
			if (vec[ench_idx] == target_ench) {
				break;
			}
		}

		out << "if (enchant_id != " << ench_idx << ") return;\n";

		if (target_constraint.attributes[0].level == -1) {
			if ((data.enchantment_mask >> 1) & (1ULL << ench_idx)) {
				out << "loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;\n";
			}
			return;
		}

		// enchantment and level
		int max_level = mc::get_max_level(target_ench);
		out << "if (nextInt(&loot_seed, " << max_level
			<< ") + 1 != " << target_constraint.attributes[0].level << ") return;\n";
	}

	void StatepredKernel::emit_entry_function_enchant_with_levels(
		std::ostream& out, const SharedEntryData& data) {
		if (data.enchantment_mask & 1) {
			out << "enchant_with_levels_function(&loot_seed, &(data["
				<< (data.enchantment_mask >> 1) << "]));";
		}
	}

	void StatepredKernel::emit_entry_function_apply_damage(
		std::ostream& out, const SharedEntryData& data, data::LootPool* pool) {
		std::string apply_damage_bitmask = create_apply_damage_item_mask(pool);
		std::string one = pool->children.size() > 32 ? "((u64)1)" : "((u32)1)";

		out << "if (" << apply_damage_bitmask << " & (" << one << " << " << data.item_idx << R"()) {
			loot_seed = (loot_seed * 25214903917 + 11) & MASK_48;
		})";
	}

	void StatepredKernel::emit_state_prediction_entry_handler(
		std::ostream& out, const SharedEntryData& data) {
		CAST_CHILD(pool, data::LootPool, entry->parent);

		for (auto& child : entry->children) {
			CAST_CHILD(func, data::LootFunctionData, child); // no nullptr

			switch (func->type) {
				case data::ENCHANT_WITH_LEVELS: {
					emit_entry_function_enchant_with_levels(out, data);
					break;
				}
				case data::ENCHANT_RANDOMLY: {
					emit_entry_function_enchant_randomly(out, data);
					break;
				}
				case data::APPLY_DAMAGE: {
					emit_entry_function_apply_damage(out, data, pool);
					break;
				}
				case data::SET_COUNT: {
					emit_entry_function_set_count(out, data);
					break;
				}
				case data::IGNORED:
				default: {
				}
			}
		}
	}

	// ------------------------------------------------

	void StatepredKernel::generate_statepred_filter(std::ostream& out) {
		out << "__device__ void statepred_filter(u64 original_state, u32 data[], u64* "
			   "result_array, u32* result_count) {\n";
		out << "u64 loot_seed = original_state;\n";
		out << "i32 calculated_count = 1; // don't know if it will satisfy constraint "
			   "requirements\n";

		CAST_CHILD(pool, data::LootPool, entry->parent);
		int pool_off = this->pool_memory_offsets[pool->child_index];
		materialize_level(pool);
		std::string item_ident = std::to_string(target_constraint.item);
		std::string arrayPlusIndex = var_name_map[item_ident];

		SharedEntryData sed(pool_off, kgen_config.bytes_per_entry, entry, combined_shared_memory);
		emit_state_prediction_entry_handler(
			out, sed); // initializes calculated_count, processes entry functions

		// if (this->target_constraint.count_range.max - this->target_constraint.count_range.min >
		// 0) {
		out << "i32 local_constraints[" << (pool->constraints.size() + 1) << "] = {0};\n";
		out << arrayPlusIndex << " = calculated_count;\n";
		out << "for (i32 roll = 0; roll < " << (pool->rolls.max - 1) << "; roll++) {\n";

		extract_data_prefix(out, pool, pool_off, true);

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
		std::string comp =
			target_constraint.count_range.min == target_constraint.count_range.max ? " == " : ">=";
		out << "if (!(" << arrayPlusIndex  << " " << comp << target_constraint.count_range.min
			<< ")) return;\n";

		// Now it's time to go back and do the full forward check:
		// - calculate min backward state advancement
		// - calculate max backward state advancement
		// - loop over the valid range of states
		int32_t min_back = 1;
		int32_t max_back = pool->get_max_lcg_advancement();

		out << "loot_seed = original_state;\n";
		out << Kernel::generate_skip("loot_seed", -min_back) << ";\n";
		// out << "#pragma unroll\n";
		out << "for (int back = 0; back < " << (max_back - min_back + 1) << "; back++) {\n";

		// TODO optimization: calculate possible roll count range and check before doing full
		// forward filter
		out << R"(
	if (forward_filter(loot_seed, data)) {
		write_result(loot_seed ^ JRAND_MULTIPLIER, result_array, result_count);
	}
)";
		out << "    " << Kernel::generate_skip("loot_seed", -1) << ";\n";
		out << "}\n"; // end of for loop

		out << "}\n\n"; // end of function
	}

	static float calculate_reduction(const StatepredKernel* sp) {
		CAST_CHILD(pool, data::LootPool, sp->entry->parent);

		uint64_t target = sp->target_constraint.count_range.min;
		uint64_t max_rolls = pool->rolls.max;
		uint64_t remainders = sp->entry->next_int_range.max - sp->entry->next_int_range.min + 1;
		float p = ((float)remainders / (float)sp->prediction_bound);

		float pre = 1.0f;

		for (const auto& attribute : sp->target_constraint.attributes) { // totally a vector
			p *= (1.0f / (float)sp->entry->get_enchant_vector_size());
			pre *= (1.0f / (float)sp->entry->get_enchant_vector_size());
			if (attribute.level != -1) {
				p *= (1.0f /
					  (float)mc::get_max_level(mc::get_enchantment_from_attribute(attribute)));
				pre *= (1.0f /
						(float)mc::get_max_level(mc::get_enchantment_from_attribute(attribute)));
			}
		}

		float q = 1 - p;
		float avg_per_roll =
			(sp->entry->get_count_range().min + sp->entry->get_count_range().max) / 2.0f;

		uint64_t required_rolls = util::min(max_rolls, static_cast<uint64_t>(ceil((target - avg_per_roll) / avg_per_roll)));

		return ((float)util::choose(max_rolls, required_rolls) * pow(p, required_rolls) *
				   pow(q, max_rolls - required_rolls)) * pre;
	}

	static double calculate_loot_probability(const StatepredKernel* sp) {
		// construct the probability loot table with modified rolls (done?)
		// construct the limited constraint set ( = target constraint) (done?)
		// call loot_prob -> return

		uint64_t remainders = sp->entry->next_int_range.max - sp->entry->next_int_range.min + 1;
		double p_orig = ((double)remainders / (double)sp->prediction_bound);
		double p = p_orig;
		for (const auto& attribute : sp->target_constraint.attributes) { // totally a vector
			p /= (double)sp->entry->get_enchant_vector_size();
			if (attribute.level != -1) {
				p /= (double)mc::get_max_level(mc::get_enchantment_from_attribute(attribute));
			}
		}

		prob::LootTable pt;
		prob::LootPool loot_pool_first {
			1, 1,
			std::vector<prob::LootEntry>({{
				{1, 1.0, static_cast<int>(sp->entry->get_count_range().min), static_cast<int>(sp->entry->get_count_range().max)}
			}})
		};
		auto rolls = dynamic_cast<data::LootPool*>(sp->entry->parent)->rolls;
		prob::LootPool loot_pool {
			rolls.max - 1, rolls.max - 1,
			std::vector<prob::LootEntry>({{
				{1, p, static_cast<int>(sp->entry->get_count_range().min), static_cast<int>(sp->entry->get_count_range().max)},
				{2, 1.0 - p, 1, 1}
			}})
		};

		pt.pools.push_back(loot_pool_first);
		pt.pools.push_back(loot_pool);

		std::vector<prob::TargetItem> target_items({{
			1, static_cast<int>(sp->target_constraint.count_range.min), sp->target_constraint.count_range.min == sp->target_constraint.count_range.max
		}});

		return prob::get_loot_probability(pt, target_items);
	}

	float StatepredKernel::heuristic() const {
		CAST_CHILD(pool, data::LootPool, entry->parent);

		uint64_t total_threads = (UINT64_C(1) << 48) / this->prediction_bound;
		uint64_t remainders = this->entry->next_int_range.max - this->entry->next_int_range.min + 1;
		uint64_t backwards = pool->get_max_lcg_advancement();

		//float reduction = calculate_reduction(this);
		//printf("old reduction = %f\n", reduction);
		double reduction = calculate_loot_probability(this);
		printf("new reduction = %f\n", reduction);

		double h = (total_threads * remainders * util::max(1.0, backwards * reduction));
		return h / (double)(1ull << 48);
	}

} // namespace kgen

/*
	heuristic = ((total_thread_count * remainders * (1 + (backwards * reduction)))) / (2 ^ 48))
	reduction = nCr(max_rolls, required_rolls) * p^{required_rolls} * (1 - p)^{max_rolls -
   required_rolls}
*/