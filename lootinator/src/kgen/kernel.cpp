#include "lootinator/kgen/kernel.hpp"
#include "lootinator/global_settings.hpp"
#include <cinttypes>
#include <fstream>

namespace kgen {
	KernelGenConfig::KernelGenConfig(mc::VersionRange version, std::string loot_table_path,
		std::string constraint_path, std::string item_map_path, bool seedcracking)
		: version(version), seedcracking(seedcracking) {
		std::ifstream f(loot_table_path);
		loot_table_json = nlohmann::json::parse(f);

		std::ifstream fin(item_map_path);
		std::string item_name;
		int ix = 0;
		while (fin >> item_name) {
			item_map[item_name] = ix++;
		}

		std::vector<loot::Constraint> constr =
			loot::parse_constraints_from_json(constraint_path.c_str(), item_map);
		std::function<bool(const loot::Constraint& a, const loot::Constraint& b)> cmp_func =
			[](const loot::Constraint& a, const loot::Constraint& b) { return a.item_equal(b); };
		merge_contraints(constr, constraints, cmp_func);

		derive_mode_from_tree();
	}

	void KernelGenConfig::traverse_and_derive(data::LootTreeNode* root, bool** edges) {
		for (auto child : root->children) {
			traverse_and_derive(child, edges);
		}

		CAST_CHILD(entry, data::LootEntry, root);
		if (entry != nullptr && !entry->children.empty()) {
			for (int i = 0; i < entry->children.size() - 1; i++) {
				CAST_CHILD(f1, data::LootFunctionData, entry->children[i]);
				CAST_CHILD(f2, data::LootFunctionData, entry->children[i + 1]);
				if (f1->type != data::IGNORED && f2->type != data::IGNORED) {
					edges[f1->type][f2->type] = true;
				}
			}
		}

		CAST_CHILD(function, data::LootFunctionData, root);
		if (function == nullptr) {
			return;
		}

		// update bytes used if function is enchant randomly
		if (function->type == data::ENCHANT_RANDOMLY) {
			int usedBits = 1 + function->enchant_randomly.enchantment_order.size();
			int usedBytes = 2 + usedBits / 32;
			if (bytes_per_entry < usedBytes) {
				bytes_per_entry = usedBytes;
			}
		} else if (function->type == data::ENCHANT_WITH_LEVELS) {
			if (bytes_per_entry < 2) {
				bytes_per_entry = 2;
			}
		}
	}

	static bool check_cycles_dfs(int vertex, const int num_functions, bool** edges, bool* visited) {
		for (int v = 0; v < num_functions; v++) {
			if (edges[vertex][v]) {
				if (visited[v]) {
					return true;
				}
				visited[v] = true;
				if (check_cycles_dfs(v, num_functions, edges, visited)) {
					return true;
				}
			}
		}
		return false;
	}

	static void update_functions(data::LootTreeNode* node, bool* removed_vec) {
		for (auto child : node->children) {
			update_functions(child, removed_vec);
			CAST_CHILD(func, data::LootFunctionData, child);
			if (func == nullptr) { 
				continue; 
			}
			removed_vec[func->type] = false;
		}
	}

	void KernelGenConfig::construct_order(bool** edges, const int num_functions, data::LootTreeNode* root) {
		// while there are unused vertices, find the root and remove it from the graph.
		// when removing, push back to function order vector

		bool* removed = new bool[num_functions];
		for (int i = 0; i < num_functions; i++) { 
			removed[i] = true; 
		}
		update_functions(root, removed);

		int remaining = 0;
		for (int i = 0; i < num_functions; i++) { 
			remaining += !removed[i]; 
		}

		while (remaining > 0) {
			for (int v = 0; v < num_functions; v++) {
				if (removed[v]) continue;

				// go back to the root, as far as you can
				int current = v;
				while (true) {
					bool found_parent = false;

					for (int v2 = 0; v2 < num_functions; v2++) {
						if (v2 != current && edges[v2][current] && !removed[v2]) {
							current = v2;
							found_parent = true;
							break;
						}
					}

					if (!found_parent) {
						break;
					}
				}

				// remove the vertex
				remaining--;
				removed[current] = true;
				// update the vector
				function_order.push_back(static_cast<data::LootFunctionType>(current));
				break;
			}
		}
	}

	void KernelGenConfig::derive_mode_from_tree() {
		data::LootTableRoot root = data::LootTableRoot(loot_table_json, item_map, version);
		bytes_per_entry = 1;
		no_fast_filter = false;

		const int NUM_FUNCTIONS = data::IGNORED;

		bool** edges = new bool*[NUM_FUNCTIONS];
		for (int i = 0; i < NUM_FUNCTIONS; i++) {
			edges[i] = new bool[NUM_FUNCTIONS];
			for (int j = 0; j < NUM_FUNCTIONS; j++) {
				edges[i][j] = false;
			}
		}

		traverse_and_derive(&root, edges);

		// "DFS"
		bool* visited = new bool[NUM_FUNCTIONS];
		for (int v = 0; v < NUM_FUNCTIONS; v++) {
			for (int v2 = 0; v2 < NUM_FUNCTIONS; v2++) 
				visited[v2] = false;
			visited[v] = true;
			if (check_cycles_dfs(v, NUM_FUNCTIONS, edges, visited)) {
				no_fast_filter = true;
			}
		}
		delete[] visited;

		if (!no_fast_filter) {
			construct_order(edges, NUM_FUNCTIONS, &root);
		}

		for (int i = 0; i < NUM_FUNCTIONS; i++) {
			delete[] edges[i];
		}
		delete[] edges;
	}

	void Kernel::fill_function_shared_mem(data::LootTreeNode* current) {
		CAST_CHILD(func, data::LootFunctionData, current);
		if (func == nullptr) {
			for (auto child : current->children) {
				fill_function_shared_mem(child);
			}
		} else {
			uint32_t size = static_cast<uint32_t>(func->shared_mem.size());
			// printf("sizeof(%ld) = %ld\n", func->id, size);
			uint32_t last = function_memory_offsets.back();

			if (func->type == data::LootFunctionType::ENCHANT_WITH_LEVELS) {
				//printf("func: %ld %ld %ld\n",
				//	func->enchant_with_levels.enchantability,
				//	func->enchant_with_levels.level.min,
				//	func->enchant_with_levels.level.max);
				combined_shared_memory.push_back(func->enchant_with_levels.enchantability);
				combined_shared_memory.push_back(func->enchant_with_levels.level.min);
				combined_shared_memory.push_back(func->enchant_with_levels.level.max);
				function_memory_offsets.push_back(last + size + 3);
			} else {
				function_memory_offsets.push_back(last + size);
			}

			for (auto i : func->shared_mem) {
				combined_shared_memory.push_back(i);
			}
		}
	}

	std::string Kernel::generate_skip(std::string var, int amount) {
		// thanks cubitect :)
		// https://github.com/Cubitect/cubiomes/blob/e61f90580cbdd883214a8054670dacae655e59c0/rng.h#L152
		uint64_t m = 1;
		uint64_t a = 0;
		uint64_t im = 0x5deece66dULL;
		uint64_t ia = 0xb;
		uint64_t k;

		for (k = amount; k; k >>= 1) {
			if (k & 1) {
				m *= im;
				a = im * a + ia;
			}
			ia = (im + 1) * ia;
			im *= im;
		}

		return var + "= (" + var + "*" + std::to_string(m) + "+" + std::to_string(a) +
			   ") & MASK_48";
	}

	void Kernel::setup_shared_memory() {
		function_memory_offsets.push_back(0);
		pool_memory_offsets.push_back(0);

		// parse shared mem for pools
		for (auto child : root_node.children) {
			CAST_CHILD(pool, data::LootPool, child);
			uint32_t size = static_cast<uint32_t>(pool->entry_lookup.size());
			uint32_t last = pool_memory_offsets.back();
			pool_memory_offsets.push_back(last + size);

			for (auto i : pool->entry_lookup) {
				combined_shared_memory.push_back(i);
			}
		}

		fill_function_shared_mem(&root_node);
		uint32_t last_pool_offset = pool_memory_offsets.back();
		//printf("%ld\n", last_pool_offset);
		int i = 0;
		for (auto& func_off : function_memory_offsets) {
			func_off += last_pool_offset;
			//printf("start_index for function %d = %d\n", i++, func_off);
		}
	}

	Kernel::Kernel(data::LootTableRoot& root_node, const kgen::KernelGenConfig& kgen_config)
		: root_node(root_node), kgen_config(kgen_config) {
		static int kernel_index = 0;
		kernel_index++;
		name = "kernel_" + std::to_string(kernel_index);
	}

	void Kernel::write_shared_definitions(std::ostream& out) {
		out << "#ifndef SHARED_DEFINITIONS\n"
			<< "//@SharedDefinitionsStart\n"
			<< "typedef " << global_settings.UNSIGNED_32_TYPE << " u32;\n"
			<< "typedef " << global_settings.SIGNED_32_TYPE << " i32;\n"
			<< "typedef " << global_settings.UNSIGNED_64_TYPE << " u64;\n"
			<< "typedef " << global_settings.SIGNED_64_TYPE << " i64;\n\n";

		out <<
			R"(#define JRAND_MULTIPLIER (0x5deece66d)
#define MASK_48 (0xffffffffffff)

__device__ inline void setSeed(u64* rand, u64 value){ *rand = (value ^ JRAND_MULTIPLIER) & MASK_48; }
__device__ inline i32 next(u64* rand, const i32 bits){ *rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48; return (i32)((i64)*rand >> (48 - bits)); }
__device__ inline i32 nextInt(u64* rand, const i32 n){ if ((n-1 & n) == 0) {u64 x = n * (u64)next(rand, 31); return (i32)((i64)x >> 31);} else {return (i32)(next(rand, 31) % n);} }
__device__ inline float nextFloat(u64* rand){ return next(rand, 24) / (float)(1 << 24); }; 
__device__ inline i32 nextIntBounded(u64* rand, const i32 min, const i32 max) {if (min >= max) {return min;} return nextInt(rand, max - min + 1) + min;}
__device__ inline i32 nextIntNoAdvance(u64 *rand, const i32 n) {if ((n-1 & n) == 0) {u64 x = n * *rand; return (i32)((i64)x >> 31);} else {return (i32)(*rand % n);}} 
__device__ inline void write_result(u64 input_seed, u64 *result_array, u32 *result_count) {result_array[atomicAdd(result_count, 1)] = input_seed;}

__device__ void enchant_with_levels_function(u64* rand, const u32* array_pointer) {
	const u32 enchantability = array_pointer[0];
	const u32 minLevel = array_pointer[1];
	const u32 maxLevel = array_pointer[2];

	// calculate effective level
	i32 level = minLevel;
	if (minLevel != maxLevel) {
		level += nextInt(rand, maxLevel - minLevel + 1);
    }
	const i32 delta = enchantability / 4 + 1;
	level += 1 + nextInt(rand, delta) + nextInt(rand, delta);
	const float amplifier = (nextFloat(rand) + nextFloat(rand) - 1.0F) * 0.15F;
	level = floor( ((float)level + (float)level * amplifier) + 0.5F );

	u32 nGroups = array_pointer[3 + level];
	if (nGroups == 0) {
        return;
    }
	*rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;

	while (nextInt(rand, 50) <= level) {
        nGroups--;
		if (nGroups == 0) {
            break;
        }
        *rand = (*rand * JRAND_MULTIPLIER + 11) & MASK_48;
		level /= 2;
	}
}

//@SharedDefinitionsEnd
#endif
)";
	}
} // namespace kgen