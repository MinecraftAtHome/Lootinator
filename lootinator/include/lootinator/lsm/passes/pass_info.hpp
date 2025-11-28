#include "lootinator/lsm/passes/loot_asserts.hpp"
#include "lootinator/lsm/passes/loot_functions.hpp"
#include "lootinator/constraint/filter.h"

namespace lsm {
    struct PassInfo {
        std::vector<mc::LootFunctionData> function_data;
        int num_assertions;
        std::vector<lsm::PoolAsserts> pool_asserts;
        const loot::PoolFilter &pool_filter;

        PassInfo(const loot::PoolFilter &pool_filter) : pool_filter(pool_filter) {
            this->num_assertions = 0;
        }
    };
}
