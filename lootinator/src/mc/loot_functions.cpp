#include "lootinator/mc/loot_functions.hpp"
#include "lootinator/utility/range.h"


namespace mc {
    // forward declarations of static functions
    static void create_list_enchant_randomly_vector(mc::LootFunctionData& lfd, const nlohmann::json &list);
    static void create_enchant_randomly_vector(mc::LootFunctionData& lfd, const std::vector<mc::Enchantment>& enchants, mc::ItemType item_type, const nlohmann::json &function);
    static void create_skip_enchant_with_levels_vector(mc::LootFunctionData& lfd, const std::vector<mc::Enchantment>& enchants, const std::string& item_name, const nlohmann::json &function);
    static int get_enchant_with_levels_groups(const std::vector<mc::Enchantment>& enchants, int level, mc::ItemType item_type, bool allow_treasure);

    mc::LootFunctionData::LootFunctionData() : type(mc::LootFunctionType::IGNORED) {}

    /**
     * Parses enchantment function data from provided json strings. If the operation fails or the function
     * is not of type `enchant_randomly` or `enchant_with_levels`, `LootFunctionData::type` is set to `IGNORED`.
     * In that case, all other contents of the returned struct should be ignored.
     */
    mc::LootFunctionData parse_loot_function_data(const loot::LootTable &loot_table, loot::lsm::Function function_ref)
    {
        mc::LootFunctionData lfd;
        lfd.function_ref = function_ref;

        int function_id = function_ref.function_id;
        int entry_id = function_ref.entry_id;
        int pool_id = function_ref.pool_id;

        const auto &entry = loot_table.data["pools"][pool_id]["entries"][entry_id];

        // safeguards
        if (!entry.contains("functions") || entry["functions"].size() <= function_id) {
            return lfd;
        }
        if (entry["type"] != "minecraft:item") {
            std::fprintf(stderr, "create_enchant_randomly_vector(): got non-item entry as input.\n");
            return lfd;
        }

        const auto& func = entry["functions"][function_ref.function_id];
        const auto& func_name = func["function"];

        std::string item_name = mc::strip_prefix(entry["name"]);
        mc::ItemType item_type = mc::string_to_item_type(item_name);
        std::string function_name = mc::strip_prefix(func["function"]); 
        std::vector<mc::Enchantment> all_enchants = mc::get_enchantments_for_version(loot_table.version_range);

        // TODO: these function names should be moved out into some sort of registry for maintainability.
        if (function_name == "enchant_randomly") {
            // some versions define available enchantments as "options", others use "enchantments"
            if (func.contains("options") && func["options"] != "#minecraft:on_random_loot") {
                create_list_enchant_randomly_vector(lfd, func["options"]);
            }
            else if (func.contains("enchantments")) {
                create_list_enchant_randomly_vector(lfd, func["enchantments"]);
            }
            else {
                // defaulting to the full list if enchantments undefined
                create_enchant_randomly_vector(lfd, all_enchants, item_type, func);
            }
        }
        else if (function_name == "enchant_with_levels") {
            create_skip_enchant_with_levels_vector(lfd, all_enchants, item_name, func);
        }
        else if (function_name == "set_count") {
            lfd.type = mc::LootFunctionType::SET_COUNT;

            util::RangeInclusive<int> range = util::RangeInclusive<int>::from_json(func["count"]);
            lfd.set_count.min = range.min;
            lfd.set_count.max = range.max;
        }
        else if (function_name == "set_damage") {
            lfd.type = mc::LootFunctionType::APPLY_DAMAGE;
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
    static void create_list_enchant_randomly_vector(mc::LootFunctionData& lfd, const nlohmann::json &list) 
    {
        for (auto& entry : list) {
            if (!entry.is_string()) {
                std::fprintf(stderr, "create_list_enchant_randomly_vector(): got non-string enchant list element, skipped.\n");
                continue;
            }
            mc::Enchantment ench = mc::string_to_enchantment(mc::strip_prefix(entry));
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
    static void create_enchant_randomly_vector(mc::LootFunctionData& lfd, const std::vector<mc::Enchantment>& enchants, mc::ItemType item, const nlohmann::json &function) 
    {
        bool allow_treasure = function.contains("treasure") ? (bool)function["treasure"] : true;

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
    static void create_skip_enchant_with_levels_vector(mc::LootFunctionData& lfd, const std::vector<mc::Enchantment>& enchants, const std::string& item_name, const nlohmann::json &function) 
    {
        if (!function.contains("levels")) {
            fprintf(stderr, "create_skip_enchant_with_levels_vector(): levels undefined in loot function, parsing skipped.\n");
            return;
        }
        util::RangeInclusive<int> levels = util::RangeInclusive<int>::from_json(function["levels"]);
        lfd.enchant_with_levels.min_level = levels.min;
        lfd.enchant_with_levels.max_level = levels.max;

        mc::ItemType item_type = mc::string_to_item_type(item_name);
        bool allow_treasure = function.contains("treasure") ? (bool)function["treasure"] : true;

        int enchantability = mc::get_enchantability(item_name);
        int max_unamplified = levels.max + 1 + (enchantability / 4) * 2;
        int min_unamplified = levels.min + 1;
        int max_effective_level = static_cast<int>(std::ceil(1.15f * max_unamplified));
        int min_effective_level = static_cast<int>(std::floor(0.85f * min_unamplified));
        lfd.enchant_with_levels.min_effective_level = min_effective_level;
        lfd.enchant_with_levels.max_effective_level = max_effective_level;

        for (int level = 0; level <= max_effective_level; level++) {
            lfd.shared_mem.push_back(get_enchant_with_levels_groups(enchants, level, item_type, allow_treasure));
        }

        lfd.type = mc::LootFunctionType::ENCHANT_WITH_LEVELS;
    }

    static int get_enchant_with_levels_groups(const std::vector<mc::Enchantment>& enchants, int level, mc::ItemType item_type, bool allow_treasure)
    {
        std::vector<mc::Enchantment> applicable;
        for (const auto& ench : enchants) {
            if (!mc::is_enchantment_applicable(ench, item_type, false)) {
                continue;
            }
            if (!allow_treasure && mc::is_treasure_enchantment(ench)) {
                continue;
            }

            for (int ench_level = mc::get_max_level(ench); ench_level >= 1; ench_level--) {
                if (!mc::is_enchantment_available_at_level(ench, ench_level, level)) {
                    continue;
                }
                applicable.push_back(ench);
            }
        }
        return mc::count_unique_groups(applicable);
    }
}
