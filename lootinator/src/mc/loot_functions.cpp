#include "lootinator/mc/loot_functions.hpp"


namespace mc {
    mc::LootFunctionData::LootFunctionData() : type(mc::LootFunctionType::IGNORED) {}

    /**
     * @warning This is only a temporary integration helper.
     * 
     * Creates shared memory data for the provided loot function. If the function is not
     * an enchantment function or the provided data is invalid, an empty vector is returned.
     */
    std::vector<int> get_shared_memory_for_function(const loot::LootTable &loot_table, const nlohmann::json &entry, const int function_id)
    {
        return parse_loot_function_data(loot_table, entry, function_id).shared_mem;
    }

    // forward declarations
    void create_list_enchant_randomly_vector(mc::LootFunctionData& lfd, const nlohmann::json &list);
    void create_enchant_randomly_vector(mc::LootFunctionData& lfd, const std::vector<mc::Enchantment>& enchants, const nlohmann::json &entry, const nlohmann::json &function);
    void create_skip_enchant_with_levels_vector(mc::LootFunctionData& lfd, const std::vector<mc::Enchantment>& enchants, const nlohmann::json &entry, const nlohmann::json &function);
    
    /**
     * Parses enchantment function data from provided json strings. If the operation fails or the function
     * is not of type `enchant_randomly` or `enchant_with_levels`, `LootFunctionData::type` is set to `IGNORED`.
     * In that case, all other contents of the returned struct should be ignored.
     */
    mc::LootFunctionData parse_loot_function_data(const loot::LootTable &loot_table, const nlohmann::json &entry, const int function_id)
    {
        mc::LootFunctionData lfd;

        // safeguards
        if (!entry.contains("functions") || entry["functions"].size() <= function_id) {
            return lfd;
        }

        const auto& func = entry["functions"][function_id];
        std::vector<mc::Enchantment> all_enchants = mc::get_enchantments_for_version(loot_table.version_range);

        if (func["function"] == "minecraft:enchant_randomly") {
            // some versions define available enchantments as "options", others use "enchantments"
            if (func.contains("options")) {
                create_list_enchant_randomly_vector(lfd, func["options"]);
            }
            else if (func.contains("enchantments")) {
                create_list_enchant_randomly_vector(lfd, func["enchantments"]);
            }
            else {
                // defaulting to the full list if enchantments undefined
                create_enchant_randomly_vector(lfd, all_enchants, entry, func);
            }
        }
        else if (func["function"] == "minecraft:enchant_with_levels") {
            create_skip_enchant_with_levels_vector(lfd, all_enchants, entry, func);
        }
        
        return lfd;
    }

    /**
     * Shared memory structure: `[max_1, max_2, max_3, ..., max_n]`, 
     * where `max_i` is the maximum level of the i-th enchantment. 
     * Enchantment indices follow the natural enchantment order inside the loot function, not enum values.
     * The order is stored outside of shared memory and used by the compiler to map enchantment contraints to function-specific indices.
     * This variant of `enchant_randomly` uses a user-provided list of applicable enchantments.
     */
    static void create_list_enchant_randomly_vector(mc::LootFunctionData& lfd, const nlohmann::json &list) {
        for (auto& entry : list) {
            if (!entry.is_string()) {
                std::fprintf(stderr, "create_list_enchant_randomly_vector(): got non-string enchant list element, skipped.\n");
                continue;
            }
            mc::Enchantment ench = mc::string_to_enchantment(entry);
            if (ench == mc::Enchantment::NO_ENCHANTMENT) {
                std::fprintf(stderr, "create_list_enchant_randomly_vector(): got unrecognized enchantment, skipped.\n");
                continue;
            }
            int max_level = mc::get_max_level(ench);
            lfd.shared_mem.push_back(max_level);
            lfd.enchant_randomly.enchantment_order.push_back(ench);
        }
        lfd.type = mc::LootFunctionType::ENCHANT_RANDOMLY;
    }

    /**
     * Shared memory structure: `[max_1, max_2, max_3, ..., max_n]`, 
     * where `max_i` is the maximum level of the i-th enchantment. 
     * Enchantment indices follow the natural enchantment order inside the loot function, not enum values.
     * The order is stored outside of shared memory and used by the compiler to map enchantment contraints to function-specific indices.
     * This variant of `enchant_randomly` uses the standard Minecraft enchantment order for the appropriate version range.
     */
    static void create_enchant_randomly_vector(mc::LootFunctionData& lfd, const std::vector<mc::Enchantment>& enchants, const nlohmann::json &entry, const nlohmann::json &function) {
        // check if "treasure" flag set
        bool allow_treasure = function.contains("treasure") ? function["treasure"] : false;
        
        if (entry["type"] != "minecraft:item") {
            std::fprintf(stderr, "create_enchant_randomly_vector(): got non-item entry as input.\n");
            return;
        }
        mc::ItemType item = mc::string_to_item_type(entry["name"]);
        if (item == mc::ItemType::NO_ITEM) {
            std::fprintf(stderr, "create_enchant_randomly_vector(): unrecognized enchantable item.\n");
            return;
        }

        // add enchantments in natural order
        for (auto ench : enchants) {
            if (!allow_treasure && mc::is_treasure_enchantment(ench)) {
                continue;
            }
            if (mc::is_enchantment_applicable(ench, item, true)) {
                int max_level = mc::get_max_level(ench);
                lfd.shared_mem.push_back(max_level);
                lfd.enchant_randomly.enchantment_order.push_back(ench);
            }
        }
        lfd.type = mc::LootFunctionType::ENCHANT_RANDOMLY;
    }

    /**
     * Shared memory structure: `[groups_1, groups_2, groups_3, ..., groups_n]`, 
     * where `count_i` is the number of mutually-exclusive enchantment groups among all applicable enchantments, e.g. `{"fortune", "silk_touch"}`, for an effective enchanting level `i`. 
     * Enchantment indices are not stored; `enchant_with_levels` output filtering is currently unsupported.
     */
    static void create_skip_enchant_with_levels_vector(mc::LootFunctionData& lfd, const std::vector<mc::Enchantment>& enchants, const nlohmann::json &entry, const nlohmann::json &function) {
        // TODO
        lfd.type = mc::LootFunctionType::ENCHANT_WITH_LEVELS;
    }
}
