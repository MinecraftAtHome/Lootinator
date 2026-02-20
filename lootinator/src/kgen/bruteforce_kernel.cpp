#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "lootinator/global_settings.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace kgen {
	void BruteforceKernel::gen_kernels(
		std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config) {

		std::ifstream f(kgen_config.loot_table_path);
		nlohmann::json loot_table_json = nlohmann::json::parse(f);

		data::LootTableRoot root =
			data::LootTableRoot(loot_table_json, kgen_config.item_map_path, kgen_config.version);

		std::vector<loot::Constraint> constr =
			loot::parse_constraints_from_json(kgen_config.constraint_path.c_str(), root.item_map);

		try {
			root.add_constraints(constr);
		} catch (std::exception& ex) {
			std::cout << ex.what() << "\n";
		}

		BruteforceKernel bk(root, kgen_config);
		bk.setup_shared_memory();
		out.push_back(bk.generate());
	}

	BruteforceKernel::BruteforceKernel(
		data::LootTableRoot& root_node, kgen::KernelGenConfig kgen_config)
		: Kernel(root_node, kgen_config) {
	}

	// -----------------------------------------------------

	ConfiguredKernel BruteforceKernel::generate() {
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
			UINT32_C(1) << 16,
			UINT32_C(1) << 19
		};
	}

	std::string BruteforceKernel::to_string() {
		std::stringstream result;
		Kernel::write_shared_definitions(result);

		generate_forward_filter(result);

		result << R"(extern "C" __global__ void )" << this->name
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

    u64 input_seed = (u64)blockIdx.x * blockDim.x + threadIdx.x + offset;
    
    if (forward_filter(input_seed, data)) {
        write_result(input_seed ^ JRAND_MULTIPLIER, result_array, result_count);
    }
}
)";
		return result.str();
	}

	void BruteforceKernel::materialize_level(data::LootTreeNode* node) {
		std::string level = dynamic_cast<data::LootPool*>(node) != nullptr ? "local_" : "global_";
		int index = 1; // one based indexing because 0 is for the junk counter
		for (const auto& constraint : node->constraints) {
			int entry_ix = 0;
			for (auto child : node->children) {
				// each entry will only have one constraint at this point (operating solely on item
				// type)
				if (child->constraints.empty()) {
					entry_ix++;
					continue;
				}
				if (child->constraints[0] == constraint) {
					break;
				}
				entry_ix++;
			}
			std::string accumulator_identifier =
				level + "constraints[" + std::to_string(index) + "]";

			int pool_idx = 0;
			for (auto child : root_node.children) {
				if (static_cast<void*>(child) == static_cast<void*>(node)) {
					break;
				}
				pool_idx++;
			}
			int offset = pool_memory_offsets[pool_idx];

			CAST_CHILD(entry, data::LootEntry, node->children[entry_ix]);
			for (int w = entry->next_int_range.min; w <= (int)entry->next_int_range.max; w++) {
				combined_shared_memory[offset + w * 3] |= (index << 8);
			}

			index++;
			std::string item_identifier = std::to_string(entry->item);
			this->var_name_map[item_identifier] = accumulator_identifier;
		}
	}

	std::string BruteforceKernel::create_forbidden_item_mask(data::LootPool* pool) {
		bool long_long = pool->children.size() > 32;
		uint64_t bitmask = 0;
		for (auto child : pool->children) {
			data::LootEntry* entry = dynamic_cast<data::LootEntry*>(child);
			if (entry->constraints.empty() && entry->item >= 0) {
				bitmask |= 1ULL << entry->item;
			}
		}
		std::string bitmask_str = (long_long ? "((u32)" : "((u64)") + std::to_string(bitmask) + ")";
		return bitmask_str;
	}

	void BruteforceKernel::emit_cuda_for_pool(
		std::ostream& out, data::LootPool* pool, int pool_idx) {
		materialize_level(pool);

		int pool_off = this->pool_memory_offsets[pool_idx];

		out << "{\n";
		if (!pool->constraints.empty()) {
			out << "i32 local_constraints[" << (pool->constraints.size() + 1) << "] = {0};\n";
		}
		out << "i32 rolls = nextIntBounded(&loot_seed, " << pool->rolls.min << ","
			<< pool->rolls.max << ");\n";
		out << "for (i32 roll = 0; roll < rolls; roll++) {\n";
		out << "int item = nextInt(&loot_seed, " << pool->get_total_weight() << ");\n";

		// u64 enchantment_mask = (static_cast<u64>(data[0 + item * 3 + 2]) << 32) | data[0 + item *
		// 3 + 1];

		// TODO can the unpacking be done with an intrinsic function?
		out << R"(u32 entry_data = data[)" << pool_off
			<< R"( + item * 3]; // min_max_count__counter_index__enchantment_count
u64 enchantment_mask = (static_cast<u64>(data[)"
			<< pool_off << R"( + item * 3 + 2]) << 32) | data[)" << pool_off
			<< R"( + item * 3 + 1]; 
u32 item_idx = entry_data >> 24; // [8b][6b][6b][4b][8b])";

		if (this->kgen_config.seedcracking) {
			std::string one = pool->children.size() > 32 ? "((u64)1)" : "((u32)1)";
			std::string forbidden_bitmask = create_forbidden_item_mask(pool);
			out << "\nif (" << forbidden_bitmask << " & (" << one
				<< " << item_idx)) return false;\n";
		}

		out << R"(
u32 min_count = (entry_data >> 18) & 0x3f; // [6b][6b][4b][8b]
u32 max_count = (entry_data >> 12) & 0x3f; // [6b][4b][8b]
u32 counter_idx = (entry_data >> 8) & 0xf; // [4b][8b]
u32 enchantment_count = entry_data & 0xff; // [8b]

i32 item_count = nextIntBounded(&loot_seed, min_count, max_count);
i32 enchant_id = enchantment_count != 0 ? nextInt(&loot_seed, enchantment_count) : 64;

bool r = (enchantment_mask & (1 << enchant_id));
u64 m = !r - 1;
loot_seed = (loot_seed * (1|(25214903917&m)) + (11&m)) & MASK_48;

local_constraints[counter_idx] += item_count;
)";
		out << "}\n";

		// check constraint sat
		for (auto& constraint : pool->constraints) {
			std::string item_ident = std::to_string(constraint.item);
			std::string arrayPlusIndex = var_name_map[item_ident];
			std::string comp =
				constraint.count_range.min == constraint.count_range.max ? " == " : ">=";
			out << "if (!(" << arrayPlusIndex << comp << constraint.count_range.min
				<< ")) return false;\n";
		}

		out << "}\n";
	}

	void BruteforceKernel::generate_forward_filter(std::ostream& out) {
		out << "__device__ bool forward_filter(u64 loot_seed, u32 data[]) {\n";
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
		out << "return true;\n}";
	}

	void BruteforceKernel::fill_function_shared_mem(data::LootTreeNode* current) {
		// this kernel doesn't really use this
		(void)current;
		return;
	}

	void BruteforceKernel::setup_entry_memory(data::LootPool* pool) {
		for (auto child2 : pool->children) {
			CAST_CHILD(entry, data::LootEntry, child2);
			uint32_t basic_info = 0;
			uint64_t enchant_mask = 0;

			// [8b]: item idx, ...[6b][6b][4b][8b]
			basic_info |= ((entry->item & 0xff) << 24) | (entry->get_count_range().min << 18) |
						  (entry->get_count_range().max << 12);
			// counters not materialized, just ignore them :)

			// find an enchant randomly thing
			data::LootFunctionData* enchant_rand_func = nullptr;
			for (auto child3 : entry->children) {
				CAST_CHILD(lf, data::LootFunctionData, child3);
				if (lf->type != data::ENCHANT_RANDOMLY) {
					continue;
				}
				enchant_rand_func = lf;
				break;
			}

			if (enchant_rand_func != nullptr) {
				basic_info |= enchant_rand_func->enchant_randomly.enchantment_order.size();

				int i = 0;
				for (auto ench : enchant_rand_func->enchant_randomly.enchantment_order) {
					int max_level = mc::get_max_level(ench);
					enchant_mask |= (max_level > 1) ? (1ULL << i) : 0;
					i++;
				}
			}

			for (int w = 0; w < entry->weight; w++) {
				combined_shared_memory.push_back(basic_info);
				combined_shared_memory.push_back(enchant_mask & 0xFFFFFFFF);
				combined_shared_memory.push_back(enchant_mask >> 32);
			}
			// printf("%u\n", static_cast<unsigned int>(combined_shared_memory.size()));
		}
	}

	void BruteforceKernel::setup_shared_memory() {
		// constraint merging thing

		function_memory_offsets.push_back(0);
		pool_memory_offsets.push_back(0);

		// parse shared mem for pools
		for (auto child : root_node.children) {
			CAST_CHILD(pool, data::LootPool, child);
			// printf("%p\n", pool);

			setup_entry_memory(pool);

			uint32_t size = static_cast<uint32_t>(combined_shared_memory.size());
			pool_memory_offsets.push_back(size);
		}
	}
} // namespace kgen