#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "lootinator/global_settings.hpp"

#include <sstream>
#include <iostream>
#include <fstream>

namespace kgen {
    void BruteforceKernel::gen_kernels(data::LootTableRoot& root_node, std::vector<ConfiguredKernel>& out, kgen::KernelGenConfig kgen_config) {
        BruteforceKernel bk(root_node, kgen_config);
        out.push_back(bk.generate());
    }

    BruteforceKernel::BruteforceKernel(data::LootTableRoot &root_node, kgen::KernelGenConfig kgen_config) : Kernel(root_node, kgen_config) {}
    
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
            UINT64_C(1) << 16,
        };
    }

    std::string BruteforceKernel::to_string() {
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

    void BruteforceKernel::materialize_level(data::LootTreeNode *node) {
        std::string level = dynamic_cast<data::LootPool *>(node) != nullptr ? "local_" : "global_";
        int index = 0;
        for (const auto &constraint : node->constraints) {
            int entry_ix = 0;
            for (auto child : node->children) {
                // each entry will only have one constraint at this point (operating solely on item type)
                if (child->constraints.empty()) {
                    entry_ix++;
                    continue;
                }
                if (child->constraints[0] == constraint) {
                    break;
                }
                entry_ix++;
            }
            std::string accumulator_identifier = level + "constraints[" + std::to_string(index) + "]";
            
            int pool_idx = 0;
            for (auto child : root_node.children) {
                if (static_cast<void*>(child) == static_cast<void*>(node)) {
                    break;
                }
                pool_idx++;
            }
            int offset = pool_memory_offsets[pool_idx];

            CAST_CHILD(entry, data::LootEntry, node->children[entry_ix]);
            for (int w = entry->next_int_range.min; w <= entry->next_int_range.max; w++) {
                combined_shared_memory[offset + w*3] |= (index << 8);
            }
            
            index++;
            std::string item_identifier = std::to_string(entry_ix);
            this->var_name_map[item_identifier] = accumulator_identifier;
        }
    }

    std::string BruteforceKernel::create_forbidden_item_mask(data::LootPool *pool) {
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

    void BruteforceKernel::emit_cuda_for_pool(std::ostream &out, data::LootPool *pool, int pool_idx) {
        materialize_level(pool);
        out << "{\n";
        if (!pool->constraints.empty()) {
            out << "i32 local_constraints[" << (pool->constraints.size() + 1) << "] = {0};\n";
        }
        out << "i32 rolls = nextIntBounded(&loot_seed, " << pool->rolls.min << "," << pool->rolls.max << ");\n";
        out << "for (i32 roll = 0; roll < rolls; roll++) {\n";
        out << "int item = data[" << this->pool_memory_offsets[pool_idx] << "+ nextInt(&loot_seed, " << pool->get_total_weight() << ")];\n";
        
        if (this->kgen_config.seedcracking) {
            std::string one = pool->children.size() > 32 ? "((u64)1)" : "((u32)1)";
            std::string forbidden_bitmask = create_forbidden_item_mask(pool);
            out << "if (" << forbidden_bitmask << " & (" << one << " << item)) return false;\n";
        }

        // TODO can the unpacking be done with an intrinsic function?
        out << 
R"(u32 entry_data = data[398 + item*3]; // min_max_count__counter_index__enchantment_count
u64 enchantment_mask = reinterpret_cast<u64*>(data)[398 + item*3 + 1];
u32 min_count = entry_data >> 24;
u32 max_count = (entry_data >> 16) & 0xff;
u32 counter_idx = (entry_data >> 8) & 0xff;
u32 enchantment_count = entry_data & 0xff;

i32 item_count = bsBoundedNextInt(&loot_seed, min_count, max_count);
i32 enchant_id = bsNextInt(&loot_seed, enchantment_count);

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
            std::string comp = constraint.count_range.min == constraint.count_range.max ? " == " : ">=";
            out << "if (!(" << arrayPlusIndex << comp << constraint.count_range.min << ")) return false;\n";
        }

        out << "}\n";
    }

    void BruteforceKernel::generate_forward_filter(std::ostream& out) {
        out << "__device__ bool forward_filter(u64 loot_seed, u32 data[]) {\n";
        int pool_idx = 0;
        for (const auto child : this->root_node.children) {
            data::LootPool *pool = dynamic_cast<data::LootPool *>(child);
            bool empty = true;
            for (int pool_idx_2 = pool_idx + 1; pool_idx_2 < this->root_node.children.size(); pool_idx_2++) {
                if (!this->root_node.children[pool_idx_2]->constraints.empty()) {
                    empty = false;
                    break;
                }
            }
            emit_cuda_for_pool(out, pool, pool_idx++);
            if (empty) break;
        }
        out << "return true;\n}";
    }

    void BruteforceKernel::fill_function_shared_mem(data::LootTreeNode* current) {
        // this kernel doesn't really use this
        return;
    }

    void BruteforceKernel::setup_entry_memory(data::LootPool* pool) {
        for (auto child2 : pool->children) {
            CAST_CHILD(entry, data::LootEntry, child2);
            uint32_t basic_info = 0;
            uint64_t enchant_mask = 0;

            basic_info |= (entry->get_count_range().min << 24) | (entry->get_count_range().max << 16);
            // counters not materialized, just ignore them :)
            
            // find an enchant randomly thing
            data::LootFunctionData* enchant_rand_func = nullptr;
            for (auto child3 : entry-> children) {
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

            combined_shared_memory.push_back(basic_info);
            combined_shared_memory.push_back(enchant_mask >> 32);
            combined_shared_memory.push_back(enchant_mask & 0xFFFF'FFFF);
        }
    }

    void BruteforceKernel::setup_shared_memory() {
        // constraint merging thing


        function_memory_offsets.push_back(0);
        pool_memory_offsets.push_back(0);

        // parse shared mem for pools
        for (auto child : root_node.children) {
            data::LootPool* pool = dynamic_cast<data::LootPool*>(child);
            
            setup_entry_memory(pool);

            uint32_t size = static_cast<uint32_t>(combined_shared_memory.size());
            pool_memory_offsets.push_back(size);
        }
    }
}