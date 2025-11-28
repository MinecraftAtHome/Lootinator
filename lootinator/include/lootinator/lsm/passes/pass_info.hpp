#include "lootinator/lsm/passes/loot_asserts.hpp"
#include "lootinator/lsm/passes/loot_functions.hpp"
#include "lootinator/constraint/filter.h"

namespace lsm {
    enum KernelStructureType {
        /**
         * This name speaks for itself.
         */
        BRUTEFORCE,

        /**
         * This is the most basic state prediction kernel type. We're predicting the state that generates
         * an item weight corresponding to one specific item.
         * The filter-on instruction needs the min and max weight that produce the item, as well as a list of loot functions that get applied to the item.
         */
        STATE_PREDICTION_WEIGHT,

        /**
         * This type of kernel reverses the weight and enchantment choice calls, iterating possible weights
         * within the range that produces the target item.
         * The filter-on needs the min and max weight producing the item, enchantment index nextInt bound, target enchantment index, as well as a list of loot functions that get applied to the item. 
         */
        ADVANCED_REVERSAL_WEIGHT_AND_ENCHANTMENT,

        /**
         * This is the most basic state prediction kernel type. We're predicting the state that generates
         * an item weight corresponding to one specific item.
         * The filter-on instruction needs the enchantment index nextInt bound, target enchantment index, enchantment level bound, enchantment level, as well as a list of loot functions that get applied to the item.
         */
        ADVANCED_REVERSAL_ENCHANTMENT_AND_LEVEL

        // TODO durability state pred
    };


    struct PassInfo {
        // for filter-on ----------------------------------------
        KernelStructureType kernel_structure_type;
        std::vector<lsm::Function> filter_on_extra_functions;
        std::vector<int> filter_on_code_data;
        // ------------------------------------------------------

        std::vector<mc::LootFunctionData> function_data;
        int num_assertions;
        std::vector<lsm::PoolAsserts> pool_asserts;
        const loot::PoolFilter &pool_filter;

        PassInfo(const loot::PoolFilter &pool_filter) : pool_filter(pool_filter) {
            this->num_assertions = 0;
        }
    };
}
