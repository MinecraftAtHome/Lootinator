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
            return create_enchant_with_levels_vector(all_enchants, entry, func);
        }
        
        return std::vector<int>();
    }


    std::vector<int> create_list_enchant_randomly_vector(const std::vector<mc::Enchantment>& enchants, const nlohmann::json &entry, const nlohmann::json &function, const nlohmann::json &list) {

    }

    std::vector<int> create_enchant_randomly_vector(const std::vector<mc::Enchantment>& enchants, const nlohmann::json &entry, const nlohmann::json &function) {

    }

    std::vector<int> create_enchant_with_levels_vector(const std::vector<mc::Enchantment>& enchants, const nlohmann::json &entry, const nlohmann::json &function) {

    }
}