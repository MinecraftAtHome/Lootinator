#include "lootinator/lsm/passes/loot_asserts.hpp"
#include "lootinator/lsm/passes/loot_functions.hpp"
#include "lootinator/constraint/filter.h"
#include "lootinator/lsm/lsm.hpp"


namespace lsm {
    struct PassInfo {
        // for filter-on ----------------------------------------
        lsm::KernelStructureType kernel_structure_type;
        std::vector<lsm::Function> filter_on_extra_functions;
        std::vector<int> filter_on_code_data;
        // ------------------------------------------------------

        std::vector<lsm::LootFunctionData> function_data;
        int num_assertions;
        std::vector<lsm::PoolAsserts> pool_asserts;
        const loot::PoolFilter &pool_filter;

        PassInfo(const loot::PoolFilter &pool_filter) : pool_filter(pool_filter) {
            this->num_assertions = 0;
        }
    };
}
