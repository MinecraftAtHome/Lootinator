#include "lootinator/kgen/secondary_bruteforce_kernel.hpp"
#include "lootinator/global_settings.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace kgen {
	void SecondaryBruteforceKernel::gen_kernels(data::LootTableRoot& root_node,
		std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config) {
		SecondaryBruteforceKernel bk(root_node, kgen_config);
		out.push_back(bk.generate());
	}

	SecondaryBruteforceKernel::SecondaryBruteforceKernel(
		data::LootTableRoot& root_node, kgen::KernelGenConfig kgen_config)
		: Kernel(root_node, kgen_config) {
	}

	// -----------------------------------------------------

	ConfiguredKernel SecondaryBruteforceKernel::generate() {
		std::ofstream fout("kernel.shm");
		for (auto v : combined_shared_memory) {
			fout << v << " ";
		}

		return ConfiguredKernel{
			this->name,
			to_string(),
			UINT64_C(1) << 48,
			UINT64_C(1) << 32,
			global_settings.THREADS_PER_BLOCK,
			combined_shared_memory,
			0,
			0,
			UINT64_C(1) << 16,
		};
	}

	std::string SecondaryBruteforceKernel::to_string() {
		std::stringstream result;
		Kernel::write_shared_definitions(result);

		generate_forward_filter(result);

		result << R"(extern "C" __global__ void )" << this->name
			   << "(u64* result_array, u32* result_count, u32* shared_mem_contents, u32 "
				  "shared_mem_contents_length, u64 offset) {";
		result <<
			R"(
    extern __shared__ u32 data[];
    if (threadIdx.x < shared_mem_contents_length) {
        for (int i = threadIdx.x; i < shared_mem_contents_length; i += blockDim.x) {
            data[i] = shared_mem_contents[i];
        }
    }
    __syncthreads();

    u64 input_seed = (u64)blockIdx.x * blockDim.x + threadIdx.x + offset;
    
    if (forward_filter(input_seed, data)) {
        write_result(input_seed ^ JRAND_MULTIPLIER, result_array, result_count);
    }
}
)";
		return result.str();
	}

	static std::string constraint_to_item_identifier(const loot::Constraint& constraint) {
		std::stringstream item_identifier;
		item_identifier << constraint.item << "_";
		if (constraint.attributes.size() > 0) {
			item_identifier << constraint.attributes[0].type;
			if (constraint.attributes[0].level != -1) {
				item_identifier << "_" << constraint.attributes[0].level;
			}
		}
		return item_identifier.str();
	}

	void SecondaryBruteforceKernel::materialize_level(data::LootTreeNode* node) {
		std::string level = dynamic_cast<data::LootPool*>(node) != nullptr ? "local_" : "global_";
		int index = 0;
		for (const auto& constraint : node->constraints) {
			std::string accumulator_identifier =
				level + "constraints[" + std::to_string(index) + "]";
			index++;
			std::string item_identifier = constraint_to_item_identifier(constraint);
			this->var_name_map[item_identifier] = accumulator_identifier;
		}
	}

	static std::string get_branchless_if_guard(
		loot::Constraint& constraint, data::LootFunctionData* lfd_enchant_randomly) {
		// easy case - no enchantment, no if guard
		if (constraint.attributes.empty()) {
			return "";
		}

		// TODO make sure this actually works
		mc::Enchantment ench = mc::get_enchantment_from_attribute(constraint.attributes[0]);
		int i = 0;
		auto& vec = lfd_enchant_randomly->enchant_randomly.enchantment_order;
		for (; i < (int)vec.size(); i++) {
			if (vec[i] == ench)
				break;
		}
		if (i == (int)vec.size()) {
			throw "messed up, need to fix :)";
		}

		// enchantment, but no level - if guard only on enchantment id
		if (constraint.attributes[0].level == -1) {
			return "item_count += (enchantment == " + std::to_string(i) + ") * ";
		}

		// enchantment & level
		return "item_count += (enchantment == " + std::to_string(i) +
			   " && level == " + std::to_string(constraint.attributes[0].level) + ") * ";
	}

	void SecondaryBruteforceKernel::emit_skip_for_entry(std::ostream& out, data::LootEntry* entry) {
		for (const auto child : entry->children) {
			data::LootFunctionData* function = dynamic_cast<data::LootFunctionData*>(child);
			switch (function->type) {
				case data::APPLY_DAMAGE: {
					out << Kernel::generate_skip("loot_seed", 1) << ";\n";
					break;
				}
				case data::SET_COUNT: {
					if (function->set_count.min != function->set_count.max) {
						out << Kernel::generate_skip("loot_seed", 1) << ";\n";
					}
					break;
				}
				case data::ENCHANT_RANDOMLY: {
					// TODO remove awful code
					auto& vec = function->enchant_randomly.enchantment_order;
					std::string bitmask = "0b";
					for (int bit = vec.size() - 1; bit >= 0; bit--) {
						int max_level = mc::get_max_level(vec[bit]);
						bitmask += (max_level > 1 ? "1" : "0");
					}
					std::string one = "1";
					if (vec.size() > 32) {
						bitmask += "ULL";
						one += "ULL";
					}
					out << "u32 eid = nextInt(&loot_seed, " << vec.size() << ");\n";
					out << "bool r = (" << bitmask << " & (" << one
						<< " << eid));\nuint64_t m = !r - 1;\n";
					out << "loot_seed = (loot_seed * (1|(25214903917&m)) + (11&m)) & MASK_48;\n";
					break;
				}
				default: {
					break;
				}
			}
		}
	}

	void SecondaryBruteforceKernel::emit_cuda_for_entry(std::ostream& out, data::LootEntry* entry) {
		// SKIP
		if (entry->constraints.empty()) {
			emit_skip_for_entry(out, entry);
			return;
		}

		// STORE
		data::LootFunctionData* ench_func = nullptr;
		out << "i32 item_count = 1;";

		for (const auto child : entry->children) {
			data::LootFunctionData* function = dynamic_cast<data::LootFunctionData*>(child);
			switch (function->type) {
				case data::APPLY_DAMAGE: {
					out << Kernel::generate_skip("loot_seed", 1);
					break;
				}
				case data::SET_COUNT: {
					if (function->set_count.min == function->set_count.max) {
						out << "item_count = " << function->set_count.min << ";";
					} else {
						int bound = function->set_count.max - function->set_count.min + 1;
						out << "item_count = " << function->set_count.min
							<< " + nextInt(&loot_seed, " << bound << ");";
					}
					break;
				}
				case data::ENCHANT_RANDOMLY: {
					ench_func = function;
					out << "i32 enchantment = nextInt(&loot_seed, "
						<< function->enchant_randomly.enchantment_order.size() << ");\n";
					// std::cout << "accessing function mem offset...\n" << function->id;
					int offset = function_memory_offsets[function->id];
					// std::cout << "...done!\n";
					out << "u32 max_level = data[" << offset << "+ enchantment];\n";
					out << "i32 level = nextIntBounded(&loot_seed, 1, max_level);\n";
					break;
				}
				default: {
					break;
				}
			}
		}

		// now update accumulators
		for (auto& constraint : entry->constraints) {
			std::string if_guard = get_branchless_if_guard(constraint, ench_func);
			std::string item_ident = constraint_to_item_identifier(constraint);
			std::string arrayPlusIndex = var_name_map[item_ident];
			out << if_guard << arrayPlusIndex << " += item_count;";
		}
	}

	void SecondaryBruteforceKernel::emit_cuda_for_pool(
		std::ostream& out, data::LootPool* pool, int pool_idx) {
		materialize_level(pool);
		out << "{\n";
		if (!pool->constraints.empty()) {
			out << "i32 local_constraints[" << pool->constraints.size() << "] = {0};\n";
		}
		out << "i32 rolls = nextIntBounded(&loot_seed, " << pool->rolls.min << ","
			<< pool->rolls.max << ");\n";
		out << "for (i32 roll = 0; roll < rolls; roll++) {\n";
		out << "int item = data[" << this->pool_memory_offsets[pool_idx] << "+ nextInt(&loot_seed, "
			<< pool->get_total_weight() << ")];\n";
		out << "switch (item) {\n";
		for (const auto child : pool->children) {
			data::LootEntry* entry = dynamic_cast<data::LootEntry*>(child);
			if (this->kgen_config.seedcracking && entry->constraints.empty()) {
				continue;
			}
			out << "case " << entry->item << ": { //" << entry->name << '\n';
			emit_cuda_for_entry(out, entry);
			out << "break;}\n";
		}
		out << "default: {return false;}\n";
		out << "}}\n";

		// check constraint sat
		for (auto& constraint : pool->constraints) {
			std::string item_ident = constraint_to_item_identifier(constraint);
			std::string arrayPlusIndex = var_name_map[item_ident];
			std::string comp =
				constraint.count_range.min == constraint.count_range.max ? " == " : ">=";
			out << "if (!(" << arrayPlusIndex << comp << constraint.count_range.min
				<< ")) return false;\n";
		}

		out << "}\n";
	}

	void SecondaryBruteforceKernel::generate_forward_filter(std::ostream& out) {
		// TODO Create check_lootseed(u64) -> bool
		//      The function should use available loot pool information combined
		//      with existing implementations of loot functions and form a full,
		//      isolated boolean check of specified constraints
		materialize_level(&root_node);

		out << "__device__ bool forward_filter(u64 loot_seed, u32 data[]) {\n";
		// out << "extern __shared__ u32 data[];\n";
		if (!this->root_node.constraints.empty()) {
			out << "int global_constraints[" << this->root_node.constraints.size() << "] = {0};\n";
		}
		int pool_idx = 0;
		for (const auto child : this->root_node.children) {
			data::LootPool* pool = dynamic_cast<data::LootPool*>(child);
			bool empty = true;
			for (int pool_idx_2 = pool_idx + 1; pool_idx_2 < (int)this->root_node.children.size();
				 pool_idx_2++) {
				if (!this->root_node.children[pool_idx_2]->constraints.empty()) {
					empty = false;
					break;
				}
			}
			emit_cuda_for_pool(out, pool, pool_idx++);
			if (empty)
				break;
		}

		// check constraint sat
		for (auto& constraint : root_node.constraints) {
			std::string item_ident = constraint_to_item_identifier(constraint);
			std::string arrayPlusIndex = var_name_map[item_ident];
			std::string comp =
				constraint.count_range.min == constraint.count_range.max ? " == " : ">=";
			out << "if (!(" << arrayPlusIndex << comp << constraint.count_range.min
				<< ")) return false;\n";
		}
		out << "return true;\n}";
	}
} // namespace kgen
