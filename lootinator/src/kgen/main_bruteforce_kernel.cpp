#include "lootinator/kgen/main_bruteforce_kernel.hpp"
#include "lootinator/global_settings.hpp"

#include <sstream>
#include <iostream>
#include <fstream>

namespace kgen {
    void MainBruteforceKernel::gen_kernels(data::LootTableRoot& root_node, std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config) {
        MainBruteforceKernel bk(root_node, kgen_config);
        out.push_back(bk.generate());
    }

    MainBruteforceKernel::MainBruteforceKernel(data::LootTableRoot &root_node, kgen::KernelGenConfig kgen_config) : Kernel(root_node, kgen_config) {}
    
    // -----------------------------------------------------
    
    ConfiguredKernel MainBruteforceKernel::generate() {
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

    std::string MainBruteforceKernel::to_string() {
        std::stringstream result;
        Kernel::write_shared_definitions(result);

        generate_forward_filter(result);

        result << 
R"(extern "C" __global__ void )" << this->name << "(u64* result_array, u32* result_count, u32* shared_mem_contents, u32 shared_mem_contents_length, u64 offset) {";
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

    void MainBruteforceKernel::materialize_level(data::LootTreeNode *node) {
        std::string level = dynamic_cast<data::LootPool *>(node) != nullptr ? "local_" : "global_";
        int index = 0;
        for (const auto &constraint : node->constraints) {
            std::string accumulator_identifier = level + "constraints[" + std::to_string(index) + "]"; 
            index++;
            std::string item_identifier = std::to_string(constraint.item);
            this->var_name_map[item_identifier] = accumulator_identifier;
        }
    }

    std::string MainBruteforceKernel::create_forbidden_item_mask(data::LootPool *pool) {
        bool long_long = pool->children.size() > 32;
        uint64_t bitmask = 0;
        for (auto child : pool->children) {
            data::LootEntry* entry = dynamic_cast<data::LootEntry*>(child);
            if (entry->constraints.empty()) {
                bitmask |= 1ULL << entry->item;
            }
        }
        std::string bitmask_str = (long_long ? "((u32)" : "((u64)") + std::to_string(bitmask) + ")";
        return bitmask_str;
    }

    void MainBruteforceKernel::emit_cuda_for_pool(std::ostream &out, data::LootPool *pool, int pool_idx) {
        materialize_level(pool);
        out << "{\n";
        if (!pool->constraints.empty()) {
            out << "i32 local_constraints[" << pool->constraints.size() << "] = {0};\n";
        }
        out << "i32 rolls = nextIntBounded(&loot_seed, " << pool->rolls.min << "," << pool->rolls.max << ");\n";
        out << "for (i32 roll = 0; roll < rolls; roll++) {\n";
        out << "int item = data[" << this->pool_memory_offsets[pool_idx] << "+ nextInt(&loot_seed, " << pool->get_total_weight() << ")];\n";
        
        if (this->kgen_config.seedcracking) {
            std::string one = pool->children.size() > 32 ? "((u64)1)" : "((u32)1)";
            std::string forbidden_bitmask = create_forbidden_item_mask(pool);
            out << "if (" << forbidden_bitmask << " & (" << one << " << item)) return false;\n";
        }

        /*
        int counters[1 + 4] = {0}; // counter[0] = junk counter

        for (r = 0; r < rolls ...) {
            int w = nextInt(398); // item ID
            int item = data[w];

            uint32_t min_max_count__counter_index__enchantment_count = data[398 + item*3];
            uint64_t enchantment_mask = reinterpret_cast<uint64_t*>(data)[398 + item*3 + 1];

            int itemCount = branchlessBoundedNextInt(min_count, max_count);

            counters[counter_idx] += item_count;
        }
        

        struct EntryData {
        uint32_t min_max_count__counter_index__enchantment_count; // 8 bits min count / 8 bits max count / 8 bits counter / 8 bits enchant count
        uint64_t enchantment_mask;
        };
        */

        out << "}\n";

        // check constraint sat
        for (auto& constraint : pool->constraints) {
            std::string item_ident = std::to_string(constraint.item);
            std::string arrayPlusIndex = var_name_map[item_ident];
            std::string comp = constraint.count_range.min == constraint.count_range.max ? " == " : ">=";
            out << "if (!(" << arrayPlusIndex << comp << constraint.count_range.min << ")) return false;\n";
        }

        out << "}\n";
    }

    void MainBruteforceKernel::generate_forward_filter(std::ostream& out) {
        // TODO Create check_lootseed(u64) -> bool
        //      The function should use available loot pool information combined
        //      with existing implementations of loot functions and form a full,
        //      isolated boolean check of specified constraints
        materialize_level(&root_node);

        out << "__device__ bool forward_filter(u64 loot_seed, u32 data[]) {\n";
        //out << "extern __shared__ u32 data[];\n";
        if (!this->root_node.constraints.empty()) {
            out << "int global_constraints[" << this->root_node.constraints.size() << "] = {0};\n";
        }
        int pool_idx = 0;
        for (const auto child : this->root_node.children) {
            data::LootPool *pool = dynamic_cast<data::LootPool *>(child);
            emit_cuda_for_pool(out, pool, pool_idx++);
        }

        // check constraint sat
        for (auto& constraint : root_node.constraints) {
            std::string item_ident = constraint_to_item_identifier(constraint);
            std::string arrayPlusIndex = var_name_map[item_ident];
            std::string comp = constraint.count_range.min == constraint.count_range.max ? " == " : ">=";
            out << "if (!(" << arrayPlusIndex << comp << constraint.count_range.min << ")) return false;\n";
        }
        out << "return true;\n}";
    }
}
