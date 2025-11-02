#include <unordered_map>

#include "lootinator/lsm/instructions.hpp"
#include "lootinator/lsm/constraints_to_lsm.hpp"
#include "lootinator/template/kernel_template.h"

namespace loot { namespace lsm {
        struct Function {
            int pool_id;
            int entry_id;
            int function_id;
        }
    
        struct Chunk {
            int offset;
            int size;
        };

        struct PassInfo {
            std::vector<Function> function_refs;
            std::vector<PoolAssertFunctionInstruction *> pool_asserts;
        }

        class SharedMem {
            public:
                std::vector<int> shared;
                
                std::vector<Chunk> pool_offset;
                std::unordered_map<Function, Chunk> function_offset;

                void add_pool(std::vector<int> &pool);
                void add_function(Function function_ref, std::vector<int> &function);
                int get_pool_start(int pool_idx);                
                int get_function_start(Function function_ref);                
            }; 

        void lsm_to_cuda(loot::LootTableConstraintList &ltcl, loot::lsm::BlockInstruction *program, std::string output_file);
        void compile_roll(loot::LootTableConstraintList &ltcl, PoolInstruction *instruction);
    }
}