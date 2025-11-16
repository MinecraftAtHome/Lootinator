#ifndef LOOTINATOR_LSM_FUNCTION_REF_H
#define LOOTINATOR_LSM_FUNCTION_REF_H

namespace lsm {
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
}

#endif