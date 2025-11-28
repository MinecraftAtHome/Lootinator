#include <iostream>
#include <cstdio>

#include "lootinator/lsm/instructions.hpp"
#include "lootinator/lsm/constraints_to_lsm.hpp"
#include "lootinator/template/kernel_template.h"
#include "lootinator/lsm/cuda/lsm_to_cuda.hpp"
#include "lootinator/lsm/passes/loot_functions.hpp"
#include "lootinator/lsm/passes/loot_asserts.hpp"
#include "nlohmann/json.hpp"

namespace lsm {
    void SharedMem::add_pool(std::vector<int> &pool) {
        Chunk c = {(int)this->shared.size(), (int)pool.size()};
        this->pool_offset.push_back(c);
        for (auto &i : pool) {
            this->shared.push_back(i);
        }
    }

    void SharedMem::add_function(const Function function_ref, std::vector<int> &function) {
        Chunk c = {
            (int)this->shared.size(), (int)function.size()
        };
        this->function_offset[function_ref] = c;
        for (auto &i : function) {
            this->shared.push_back(i);
        }
    }

    int SharedMem::get_pool_start(int pool_idx) {
        Chunk chunk = this->pool_offset[pool_idx];
        return chunk.offset;
    }

    int SharedMem::get_function_start(Function function_ref) {
        Chunk chunk = this->function_offset[function_ref];
        return chunk.offset;
    }                

    SharedMem create_shared_memory(loot::LootTableConstraintList &ltcl) {
        SharedMem shared_mem;

        for (auto &pool : ltcl.loot_table.precomputed_loot) {
            shared_mem.add_pool(pool);
        }

        return shared_mem;
    }

    void compile_roll(loot::LootTableConstraintList &ltcl, PoolInstruction *instruction) {
        int pool_idx = instruction->id;
        const nlohmann::json &pool = ltcl.loot_table.data["pools"][pool_idx];
        util::RangeInclusive<std::uint32_t> roll_range = util::RangeInclusive<std::uint32_t>::from_json(pool["rolls"]);     

        // NOTE: in the future we will probably want to factor this out into a function that automatically computes the lcg values needed for the advancement
        FunctionInstruction *lcg_advance = dynamic_cast<FunctionInstruction *>(instruction->children[0]);
        for (int i = 0; i < lcg_advance->args[0]; i++) {        
            fprintf(stdout, "next(rand);\n");
        }         

        RollInstruction *roll_block = dynamic_cast<RollInstruction *>(instruction->children[1]);
        fprintf(stdout, "int rolls = nextIntNoAdvance(rand, %d + %d - 1) + %d;\n", roll_range.min, roll_range.max, roll_range.min);
        fprintf(stdout, "for (int r = 0; r < rolls; r++) {\n");
        fprintf(stdout, "    int item = shared_mem_items[nextInt(rand, %d)];\n", ltcl.loot_table.total_weights[pool_idx]);
        fprintf(stdout, "    switch (item) {\n");
        for (auto &child : roll_block->children) {
            CaseInstruction *case_block = dynamic_cast<CaseInstruction *>(child);
            fprintf(stdout, "        case %d:\n", case_block->item);
        }
        fprintf(stdout, "    }\n");
        fprintf(stdout, "}\n");
    }                   

    void lsm_to_cuda(loot::LootTableConstraintList &ltcl, Program &program, std::string output_file) {
        SharedMem smem = create_shared_memory(ltcl);
        program.main_block->debug(0);

        // pass 1
        program.main_block->compile_pass1(ltcl, program.pass_info.function_data);

        for (auto &function : program.pass_info.function_data) {
            if (function.type == mc::LootFunctionType::IGNORED) {
                continue;
            }
            std::cout << function.type << "\n"; 
            util::DebugStruct(std::cout, "Function")
                .add("pool_id", function.function_ref.pool_id)
                .add("entry_id", function.function_ref.entry_id)
                .add("function_id", function.function_ref.function_id)
                .finish();
            std::cout << "\n";
            smem.add_function(function.function_ref, function.shared_mem);
        }
        
        // pass 2
        program.main_block->compile_pass2((void *)&program.pass_info.pool_asserts, program.pass_info.num_assertions);

        for (auto &assert : program.pass_info.pool_asserts) {
            assert.debug();
        }

        std::cout << "===============\n";
    }   
}
