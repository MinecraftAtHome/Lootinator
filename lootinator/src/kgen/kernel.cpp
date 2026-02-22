#include "lootinator/kgen/kernel.hpp"
#include "lootinator/global_settings.hpp"
#include <cinttypes>
#include <fstream>

namespace kgen {
	KernelGenConfig::KernelGenConfig(
			mc::VersionRange version,
			std::string loot_table_path,
			std::string constraint_path,
			std::string item_map_path,
			bool seedcracking
	) : version(version), seedcracking(seedcracking) {
		std::ifstream f(loot_table_path);
		loot_table_json = nlohmann::json::parse(f);

		std::ifstream fin(item_map_path);
		std::string item_name;
		int ix = 0;
		while (fin >> item_name) {
			item_map[item_name] = ix++;
		}

		std::vector<loot::Constraint> constr = loot::parse_constraints_from_json(constraint_path.c_str(), item_map);
		std::function<bool(const loot::Constraint& a, const loot::Constraint& b)> cmp_func =
			[](const loot::Constraint& a, const loot::Constraint& b) { return a.item_equal(b); };
		merge_contraints(constr, constraints, cmp_func);
	}

	void Kernel::fill_function_shared_mem(data::LootTreeNode* current) {
		CAST_CHILD(func, data::LootFunctionData, current);
		if (func == nullptr) {
			for (auto child : current->children) {
				fill_function_shared_mem(child);
			}
		} else {
			uint32_t size = static_cast<uint32_t>(func->shared_mem.size());
			uint32_t last = function_memory_offsets.back();
			function_memory_offsets.push_back(last + size);

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
		for (auto& func_off : function_memory_offsets) {
			func_off += last_pool_offset;
		}
	}

	Kernel::Kernel(data::LootTableRoot& root_node, kgen::KernelGenConfig kgen_config)
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

__device__ inline i32 bsNextInt(u64 *rand, const i32 n) {
  bool r = (n - 1 & n);
  bool r2 = n - 1;
  u64 m = r - 1;
  u64 m2 = !r2 - 1;
  *rand = (*rand * (1|(25214903917&m2)) + (11&m2)) & MASK_48;
  u64 x = ((m & n) | r) * (*rand >> 17);
  return ((x&m2) >> (31&m)) % (n + (4294960000&m));
}
__device__ inline i32 bsBoundedNextInt(u64 *rand, const i32 min, const i32 max) {
  return min + bsNextInt(rand, max - min + 1);
}
//@SharedDefinitionsEnd
#endif
)";
	}
} // namespace kgen