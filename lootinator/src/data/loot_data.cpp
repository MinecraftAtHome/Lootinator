/*
    LootTableRoot
    LootPool
*/

#include "lootinator/data/loot_data.hpp"
#include "nlohmann/json.hpp"

namespace data {
	LootTableRoot::LootTableRoot(const nlohmann::json &json, mc::VersionRange version) {
        this->parent = nullptr;
        this->version = version;
        auto pools = json["pools"];
        for (auto &pool : pools) {
            this->children.push_back(new LootPool((LootTreeNode *)this, pool));
        }
    }

    LootPool::LootPool(LootTreeNode *parent, const nlohmann::json &json) {
        this->parent = parent;
        this->rolls = RangeInclusive<std::uint32_t>::from_json(json["rolls"]);
        int index = 0;
        for (auto &entry : json["entries"]) {
            this->children.push_back(new LootEntry((LootTreeNode *)this, entry, index));
            for (int w = 0; w < entry.contains("weight") ? (int)entry["weight"] : 1; w++) {
                this->entry_lookup.push_back(index);
            }
            index++;
        }   
    }
}