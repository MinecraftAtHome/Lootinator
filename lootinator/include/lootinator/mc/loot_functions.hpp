#ifndef LOOTINATOR_MC_LOOT_FUNCTIONS_H
#define LOOTINATOR_MC_LOOT_FUNCTIONS_H

#include "lootinator/loot_table.h"
#include "lootinator/lsm/function_ref.hpp"
#include "lootinator/utility/range.h"


namespace mc {
    enum LootFunctionType {
        ENCHANT_RANDOMLY,
        ENCHANT_WITH_LEVELS,
        SET_COUNT,
        IGNORED
    };
    
    struct SetCountData {
        int min;
        int max;
    };

    struct EnchantRandomlyData {
        std::vector<mc::Enchantment> enchantment_order;
    };

    struct EnchantWithLevelsData {
        int min_level;
        int max_level;
        int min_effective_level;
        int max_effective_level;
        // not storing enchantments, function output is skipped
    };

    struct LootFunctionData {
        std::vector<int> shared_mem;
        LootFunctionType type;
        loot::lsm::Function function_ref;

        // FIXME ideally use union
        EnchantRandomlyData enchant_randomly;
        EnchantWithLevelsData enchant_with_levels;
        SetCountData set_count;

        LootFunctionData();
    };

    std::vector<int> get_shared_memory_for_function(const loot::LootTable &loot_table, const nlohmann::json &entry, const int function_id);
    mc::LootFunctionData parse_loot_function_data(const loot::LootTable &loot_table, loot::lsm::Function function_ref);
}

#endif

