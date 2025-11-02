#ifndef LOOTINATOR_LOOT_TABLE_H
#define LOOTINATOR_LOOT_TABLE_H

#include "nlohmann/json.hpp"
#include "lootinator/mc/minecraft.hpp"
#include <vector>
#include <string>

namespace loot {
	struct LootTable {
        mc::VersionRange version_range;
        nlohmann::json data;
        std::vector<std::string> item_names;
        std::vector<std::vector<int>> precomputed_loot;
        std::vector<int> total_weights;

        LootTable(const char* loot_table_json_filepath, const mc::VersionRange version_range);
        int find_item_name(const std::string& item_name) const;

    private:
        void add_item_name(const std::string& item_name);
        void map_item_names();
        void precompute_entry_indices();
    };

    mc::AttributeCategory get_loot_function_attribute_category(const std::string& function_name);
}

#endif