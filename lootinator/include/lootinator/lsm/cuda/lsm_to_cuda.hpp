#ifndef LOOTINATOR_LSM_CUDA_LSM_TO_CUDA_H
#define LOOTINATOR_LSM_CUDA_LSM_TO_CUDA_H

#include <unordered_map>

#include "lootinator/lsm/instructions.hpp"
#include "lootinator/lsm/constraints_to_lsm.hpp"
#include "lootinator/template/kernel_template.h"
#include "lootinator/lsm/function_ref.hpp"

namespace lsm {
    struct Chunk {
        int offset;
        int size;
    };

    struct PassInfo {
        std::vector<lsm::Function> function_refs;
        std::vector<lsm::PoolAssertFunctionInstruction *> pool_asserts;
    };

    class SharedMem {
        public:
            std::vector<int> shared;
            
            std::vector<Chunk> pool_offset;
            std::unordered_map<Function, Chunk, FunctionHash> function_offset;

            void add_pool(std::vector<int> &pool);
            void add_function(Function function_ref, std::vector<int> &function);
            int get_pool_start(int pool_idx);                
            int get_function_start(Function function_ref);                
    };

    void lsm_to_cuda(loot::LootTableConstraintList &ltcl, lsm::BlockInstruction *program, std::string filename);
    void compile_roll(loot::LootTableConstraintList &ltcl, PoolInstruction *instruction);
}
#endif