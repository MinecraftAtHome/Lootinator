#include "lootinator/mc/loot_functions.hpp"

namespace mc {
    /**
     * Creates shared memory data for the provided loot function. If the function is not
     * an enchantment function or the provided data is invalid, an empty vector is returned.
     * The generated vector's structure 
     */
    std::vector<int> get_shared_memory_for_function(const loot::LootTable &loot_table, const nlohmann::json &entry, const int function_id)
    {
        // safeguards
        if (!entry.contains("functions") || entry["functions"].size() <= function_id) {
            return std::vector<int>();
        }

        const auto& func = entry["functions"][function_id];
        std::vector<mc::Enchantment> all_enchants = mc::get_enchantments_for_version(loot_table.version_range);

        if (func["function"] == "minecraft:enchant_randomly") {
            if (func.contains("options")) {
                return create_list_enchant_randomly_vector(all_enchants, entry, func, func["options"]);
            }
            else if (func.contains("enchantments")) {
                return create_list_enchant_randomly_vector(all_enchants, entry, func, func["enchantments"]);
            }
            else {
                return create_enchant_randomly_vector(all_enchants, entry, func);
            }
        }
        else if (func["function"] == "minecraft:enchant_with_levels") {
            return create_skip_enchant_with_levels_vector(all_enchants, entry, func);
        }
        
        return std::vector<int>();
    }


    /**
     * Shared memory structure: `[max_1, max_2, max_3, ..., max_n]`, 
     * where `max_i` is the maximum level of the i-th enchantment. 
     * Enchantment indices follow the natural enchantment order inside the loot function, not enum values.
     * The order is stored outside of shared memory and used by the compiler to map enchantment contraints to function-specific indices.
     * This variant of `enchant_randomly` uses a user-provided list of applicable enchantments.
     */
    static std::vector<int> create_list_enchant_randomly_vector(const std::vector<mc::Enchantment>& enchants, const nlohmann::json &entry, const nlohmann::json &function, const nlohmann::json &list) {
        // TODO
    }

    /**
     * Shared memory structure: `[max_1, max_2, max_3, ..., max_n]`, 
     * where `max_i` is the maximum level of the i-th enchantment. 
     * Enchantment indices follow the natural enchantment order inside the loot function, not enum values.
     * The order is stored outside of shared memory and used by the compiler to map enchantment contraints to function-specific indices.
     * This variant of `enchant_randomly` uses the standard Minecraft enchantment order for the appropriate version range.
     */
    static std::vector<int> create_enchant_randomly_vector(const std::vector<mc::Enchantment>& enchants, const nlohmann::json &entry, const nlohmann::json &function) {
        // TODO
    }

    /**
     * Shared memory structure: `[groups_1, groups_2, groups_3, ..., groups_n]`, 
     * where `count_i` is the number of mutually-exclusive enchantment groups among all applicable enchantments, e.g. `{"fortune", "silk_touch"}`, for an effective enchanting level `i`. 
     * Enchantment indices are not stored; `enchant_with_levels` output filtering is currently unsupported.
     */
    static std::vector<int> create_skip_enchant_with_levels_vector(const std::vector<mc::Enchantment>& enchants, const nlohmann::json &entry, const nlohmann::json &function) {
        // TODO
    }
}