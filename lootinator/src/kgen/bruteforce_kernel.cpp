#include "lootinator/kgen/bruteforce_kernel.hpp"
#include "lootinator/global_settings.hpp"

#include <sstream>
#include <iostream>
#include <fstream>

namespace kgen {
    void BruteforceKernel::gen_kernels(data::LootTableRoot& root_node, std::vector<ConfiguredKernel>& out) {
        BruteforceKernel bk(root_node);
        out.push_back(bk.generate());
    }

    BruteforceKernel::BruteforceKernel(data::LootTableRoot &root_node) : Kernel(root_node) {}
    
    // -----------------------------------------------------
    
    ConfiguredKernel BruteforceKernel::generate() {
        std::ofstream fout("kernel.shm");//TODO UNDO
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

    void BruteforceKernel::generate_helper_functions(std::ostream& out) const {
        // TODO Create loot function implementations based on shared memory.
        //      Each loot function should have a skip variant and a save variant
        //      to let the generator choose the intended effect of the function.
        // - skip signature: void skip_loot_function_name(u64*)
        // - save signature: SaveType save_loot_function_name(u64*)
    }

    static std::string constraint_to_item_identifier(const loot::Constraint &constraint) {
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

    void BruteforceKernel::materialize_level(data::LootTreeNode *node) {
        std::string level = dynamic_cast<data::LootPool *>(node) != nullptr ? "local_" : "global_";
        int index = 0;
        for (const auto &constraint : node->constraints) {
            std::string accumulator_identifier = level + "constraints[" + std::to_string(index) + "]"; 
            index++;
            std::string item_identifier = constraint_to_item_identifier(constraint);
            this->var_name_map[item_identifier] = accumulator_identifier;
        }
    }

    static std::string get_if_guard(loot::Constraint& constraint, data::LootFunctionData* lfd_enchant_randomly) {
        // easy case - no enchantment, no if guard
        if (constraint.attributes.empty()) {
            return "";
        }

        // TODO make sure this actually works
        mc::Enchantment ench = mc::get_enchantment_from_attribute(constraint.attributes[0]);
        int i = 0;
        auto& vec = lfd_enchant_randomly->enchant_randomly.enchantment_order;
        for (; i < vec.size(); i++) {
            if (vec[i] == ench) break;
        }
        if (i == vec.size()) {
            throw "messed up, need to fix :)";
        }

        // enchantment, but no level - if guard only on enchantment id
        if (constraint.attributes[0].level == -1) {
            return "if (enchantment == " + std::to_string(i) + ") ";
        }

        // enchantment & level
        return "if (enchantment == " + std::to_string(i) + " && level == " + std::to_string(constraint.attributes[0].level) + ") ";
    }

    void BruteforceKernel::emit_cuda_for_entry(std::ostream &out, data::LootEntry *entry) {
        // entry->constraints
        // std::vector<LootFunction *> = dynamic_cast<std::vector<LootFunction *>>(entry->children);
        data::LootFunctionData* ench_func = nullptr;

        out << "i32 item_count = 1;";

        for (const auto child : entry->children) {            
            data::LootFunctionData *function = dynamic_cast<data::LootFunctionData *>(child);
            switch (function->type) {
                case data::APPLY_DAMAGE: {
                    out << Kernel::generate_skip("loot_seed", 1);
                    break;
                }
                case data::SET_COUNT: {
                    if (function->set_count.min == function->set_count.max) {
                        out << "item_count = " << function->set_count.min << ";";
                    }
                    else {
                        int bound = function->set_count.max - function->set_count.min + 1;
                        out << "item_count = " << function->set_count.min << " + nextInt(&loot_seed, " << bound << ");";
                    }
                    break;
                }
                case data::ENCHANT_RANDOMLY: {
                    ench_func = function;
                    out << "i32 enchantment = nextInt(&loot_seed, " << function->enchant_randomly.enchantment_order.size() << ");\n";
                    std::cout << "accessing function mem offset...\n" << function->id;
                    int offset = function_memory_offsets[function->id];
                    std::cout << "...done!\n";
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
            std::string if_guard = get_if_guard(constraint, ench_func);
            std::string item_ident = constraint_to_item_identifier(constraint);
            std::string arrayPlusIndex = var_name_map[item_ident];
            out << if_guard << arrayPlusIndex << " += item_count;"; 
        }
    }

    void BruteforceKernel::emit_cuda_for_pool(std::ostream &out, data::LootPool *pool, int pool_idx) {
        materialize_level(pool);
        out << "{\n";
        if (!pool->constraints.empty()) {
            out << "i32 local_constraints[" << pool->constraints.size() << "] = {0};\n";
        }
        out << "i32 rolls = nextIntBounded(&loot_seed, " << pool->rolls.min << "," << pool->rolls.max << ");\n";
        out << "for (i32 roll = 0; roll < rolls; roll++) {\n";
        out << "int item = data[" << this->pool_memory_offsets[pool_idx] << "+ nextInt(&loot_seed, " << pool->get_total_weight() << ")];\n";
        out << "switch (item) {\n";
        for (const auto child : pool->children) {
            data::LootEntry *entry = dynamic_cast<data::LootEntry *>(child);
            out << "case " << entry->item << ": { //" << entry->name << '\n';
            emit_cuda_for_entry(out, entry); 
            out << "break;}\n";

            // TODO POOL SKIPPING
            //if ()
        }
        out << "}}\n";

         // check constraint sat
        for (auto& constraint : pool->constraints) {
            std::string item_ident = constraint_to_item_identifier(constraint);
            std::string arrayPlusIndex = var_name_map[item_ident];
            std::string comp = constraint.count_range.min == constraint.count_range.max ? " == " : ">=";
            out << "if (!(" << arrayPlusIndex << comp << constraint.count_range.min << ")) return false;\n";
        }

        out << "}\n";
    }

    void BruteforceKernel::generate_forward_filter(std::ostream& out) {
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
