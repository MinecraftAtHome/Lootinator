#ifndef LOOTINATOR_MC_LOOT_FUNCTIONS_H
#define LOOTINATOR_MC_LOOT_FUNCTIONS_H

#include "lootinator/loot_table.h"

namespace mc {
    std::vector<int> get_shared_memory_for_function(const loot::LootTable &loot_table, const nlohmann::json &entry, const int function_id);
}

#endif

