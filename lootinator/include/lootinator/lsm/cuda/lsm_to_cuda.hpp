#include <unordered_map>

#include "lootinator/lsm/instructions.hpp"
#include "lootinator/lsm/constraints_to_lsm.hpp"
#include "lootinator/template/kernel_template.h"

namespace loot { namespace lsm {
        struct Function {
            int pool_id;
            int entry_id;
            int function_id;
        
            bool operator ==(const Function &other) const {
                return other.pool_id == this->pool_id && other.entry_id == this->entry_id && other.function_id == this->function_id;
            }
        };

        struct FunctionHash {
            std::size_t operator()(const Function &func) const {
                return std::hash<int>()(func.pool_id) ^ (std::hash<int>()(func.entry_id) << 1) ^ (std::hash<int>()(func.function_id) << 2); 
            }
        };
        
        struct Chunk {
            int offset;
            int size;
        };

        struct PassInfo {
            std::vector<Function> function_refs;
            std::vector<PoolAssertFunctionInstruction *> pool_asserts;
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

        void lsm_to_cuda(loot::LootTableConstraintList &ltcl, loot::lsm::BlockInstruction *program, std::string output_file);
        void compile_roll(loot::LootTableConstraintList &ltcl, PoolInstruction *instruction);
    }
}
