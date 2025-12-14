#include "lootinator/data/loot_data.hpp"


#include <fstream>

namespace data {
    mc::VersionRange LootTreeNode::get_version() {
        LootTreeNode* next = this;
        while (next->parent != nullptr) {
            next = next->parent;
        }
        return dynamic_cast<LootTableRoot*>(next)->version;
    }

    LootTreeNode::~LootTreeNode() {
        for (auto& child : children) {
            delete child;
        }
    }

    int LootTreeNode::get_item_index(const std::string &item_name) const {
        if (parent != nullptr) {
            return parent->get_item_index(item_name);
        }
        throw "illegal state in LootTreeNode::get_item_index - parent was nullptr";
    }

    LootEntry::LootEntry(LootTreeNode *parent, const nlohmann::json &json) {
        this->parent = parent;

        // entry parsing
        weight = json.contains("weight") ? json["weight"] : 1;

        if (json["type"] == "minecraft:empty") {
            // empty entry
            name = "";
            type = mc::ItemType::NO_ITEM;
            item = -1;
        }
        else {
            // item entry
            name = json["name"];
            type = mc::string_to_item_type(name);
            item = get_item_index(name);
        }

        // loot functions
        if (json.contains("functions")) {
            for (auto& function_json : json["functions"]) {
                children.push_back(new data::LootFunctionData(this, function_json));
            }
        }
    }

	LootTableRoot::LootTableRoot(const nlohmann::json &json, const std::string& item_map_filepath, mc::VersionRange version) {
        this->parent = nullptr;
        this->version = version;

        std::ifstream fin(item_map_filepath);
        std::string item_name;
        int ix = 0;
        while (fin >> item_name) {
            this->item_map[item_name] = ix++;
        }

        auto pools = json["pools"];
        for (auto &pool : pools) {
            this->children.push_back(new LootPool(this, pool));
        }
    }

    int LootTableRoot::get_item_index(const std::string &item_name) const {
        if (item_map.find(item_name) != item_map.end()) {
            return item_map.at(item_name);
        }
        return -1;
    }

    LootPool::LootPool(LootTreeNode *parent, const nlohmann::json &json) : rolls(util::RangeInclusive<std::uint32_t>::from_json(json["rolls"])) {
        this->parent = parent;
        int index = 0;
        for (auto &entry : json["entries"]) {
            this->children.push_back(new LootEntry(this, entry));
            for (int w = 0; w < entry.contains("weight") ? (int)entry["weight"] : 1; w++) {
                this->entry_lookup.push_back(index);
            }
            index++;
        }   
    }

    // -----------------------------------------------------------------
    // loot function data

    LootFunctionData::LootFunctionData(LootTreeNode *parent, const nlohmann::json &json) {
        this->parent = parent;

        // function parsing
        std::string function_name = mc::strip_prefix(json["function"]); 

        if (function_name == "enchant_randomly") {
            // some versions define available enchantments as "options", others use "enchantments"
            if (json.contains("options") && json["options"] != "#minecraft:on_random_loot") {
                create_list_enchant_randomly(json["options"]);
            }
            else if (json.contains("enchantments")) {
                create_list_enchant_randomly(json["enchantments"]);
            }
            else {
                // defaulting to the full list if enchantments undefined
                create_enchant_randomly(json);
            }
        }
        else if (function_name == "enchant_with_levels") {
            create_enchant_with_levels(json);
        }
        else if (function_name == "set_count") {
            type = data::LootFunctionType::SET_COUNT;
            set_count = util::RangeInclusive<uint32_t>::from_json(json["count"]);
        }
        else if (function_name == "set_damage") {
            type = data::LootFunctionType::APPLY_DAMAGE;
        }
    }

    // function parsing helpers

    /**
     * Shared memory structure: `[max_1, max_2, max_3, ..., max_n]`, 
     * where `max_i` is the maximum level of the i-th enchantment. 
     * Enchantment indices follow the natural enchantment order inside the loot function, not enum values.
     * The order is stored outside of shared memory and used by the compiler to map enchantment contraints to function-specific indices.
     * This variant of `enchant_randomly` uses a user-provided list of applicable enchantments.
     */
    void LootFunctionData::create_list_enchant_randomly(const nlohmann::json &list) 
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
            shared_mem.push_back(max_level);
            enchant_randomly.enchantment_order.push_back(ench);
        }
        type = data::LootFunctionType::ENCHANT_RANDOMLY;
    }

    /**
     * Shared memory structure: `[max_1, max_2, max_3, ..., max_n]`, 
     * where `max_i` is the maximum level of the i-th enchantment. 
     * Enchantment indices follow the natural enchantment order inside the loot function, not enum values.
     * The order is stored outside of shared memory and used by the compiler to map enchantment contraints to function-specific indices.
     * This variant of `enchant_randomly` uses the standard Minecraft enchantment order for the appropriate version range.
     */
    void LootFunctionData::create_enchant_randomly(const nlohmann::json &function) 
    {
        data::LootEntry* entry = dynamic_cast<data::LootEntry*>(parent);
        std::vector<mc::Enchantment> all_enchants = mc::get_enchantments_for_version(get_version());
        bool allow_treasure = function.contains("treasure") ? (bool)function["treasure"] : true;

        // add enchantments in natural order
        for (auto ench : all_enchants) {
            if (!allow_treasure && mc::is_treasure_enchantment(ench)) {
                continue;
            }
            if (mc::is_enchantment_applicable(ench, entry->type, true)) {
                int max_level = mc::get_max_level(ench);
                shared_mem.push_back(max_level);
                enchant_randomly.enchantment_order.push_back(ench);
            }
        }
        type = data::LootFunctionType::ENCHANT_RANDOMLY;
    }

    /**
     * LootFunctionData::create_enchant_with_levels helper. Returns the number of unique enchantment groups for a given item and a enchant_with_levels effective level. 
     */
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

    /**
     * Shared memory structure: `[groups_1, groups_2, groups_3, ..., groups_n]`, 
     * where `count_i` is the number of mutually-exclusive enchantment groups among all applicable enchantments, e.g. `{"fortune", "silk_touch"}`, for an effective enchanting level `i`. 
     * Enchantment indices are not stored; `enchant_with_levels` output filtering is currently unsupported.
     */
    void LootFunctionData::create_enchant_with_levels(const nlohmann::json &function) 
    {
        data::LootEntry* entry = dynamic_cast<data::LootEntry*>(parent);
        std::vector<mc::Enchantment> all_enchants = mc::get_enchantments_for_version(get_version());
        bool allow_treasure = function.contains("treasure") ? (bool)function["treasure"] : true;

        if (!function.contains("levels")) {
            fprintf(stderr, "create_skip_enchant_with_levels_vector(): levels undefined in loot function, parsing skipped.\n");
            return;
        }
        enchant_with_levels.level = util::RangeInclusive<uint32_t>::from_json(function["levels"]);

        enchant_with_levels.enchantability = mc::get_enchantability(entry->name);
        uint32_t max_unamplified = enchant_with_levels.level.max + 1 + (enchant_with_levels.enchantability / 4) * 2;
        uint32_t effective_max = static_cast<uint32_t>(std::ceil(1.15f * max_unamplified));
        //uint32_t min_unamplified = enchant_with_levels.level.min + 1;
        //uint32_t effective_min = static_cast<uint32_t>(std::floor(0.85f * min_unamplified));

        for (uint32_t level = 0; level <= effective_max; level++) {
            shared_mem.push_back(get_enchant_with_levels_groups(all_enchants, level, entry->type, allow_treasure));
        }

        type = data::LootFunctionType::ENCHANT_WITH_LEVELS;
    }
}


