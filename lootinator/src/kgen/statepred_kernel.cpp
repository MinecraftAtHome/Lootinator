#include "lootinator/kgen/statepred_kernel.hpp"

#include <iostream>

namespace kgen {
	static void traverse_and_build(std::vector<ConfiguredKernel>& out, data::LootTableRoot& root,
		kgen::KernelGenConfig kgen_config, data::LootTreeNode* node) {
		for (auto child : node->children) {
			traverse_and_build(out, root, kgen_config, child);
		}

		for (auto& constraint : node->constraints) {
			StatepredKernel spk(root, node, constraint, kgen_config);
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

	kgen::StatepredKernel::StatepredKernel(data::LootTableRoot& root_node,
		data::LootEntry* target_entry, loot::Constraint& target_constraint,
		const kgen::KernelGenConfig& kgen_config)
		: BruteforceKernel(root_node, kgen_config) {
	}

	ConfiguredKernel kgen::StatepredKernel::generate() {
		uint32_t threads_per_batch = (UINT64_C(1) << 32);
		uint32_t total_threads = (UINT64_C(1) << 48) / this->predicition_bound;
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
		std::stringsgtream result;
		Kernel::write_shared_definitions(result);

		BruteforceKernel::generate_forward_filter(result);

		data::LootPool* pool = dynamic_cast<data::LootPool*>(this->entry->parent);

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

	u64 tid = (u64)blockDim.x * blockIdx.x + threadIdx.x + offset;
	u32 lower17 = tid & ((1ull << 17) - 1);
	u64 upper31 = (tid >> 17) * )"
			   << pool->entry_lookup.size() << R"(
	u64 state = (upper31 << 17) | lower17;
}
)";
		return result.str();
	}

	void kgen::StatepredKernel::emit_cuda_for_pool(
		std::ostream& out, data::LootPool* pool, int pool_idx) {
	}
} // namespace kgen
