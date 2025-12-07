#include "lootinator/data/loot_data.hpp"

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
            name = "";
            type = mc::ItemType::NO_ITEM;
            item = -1;
        }
        else {
            name = json["name"];
            type = mc::string_to_item_type(name);
            item = get_item_index(name);
        }

        // todo loot functions etc
    }

    LootFunctionData::LootFunctionData(LootTreeNode *parent, const nlohmann::json &json) {
        this->parent = parent;

        // function parsing
    }

	LootTableRoot::LootTableRoot(const nlohmann::json &json, mc::VersionRange version) {
        this->parent = nullptr;
        this->version = version;
        auto pools = json["pools"];
        for (auto &pool : pools) {
            this->children.push_back(new LootPool((LootTreeNode *)this, pool));
        }
    }

    LootPool::LootPool(LootTreeNode *parent, const nlohmann::json &json) : rolls(util::RangeInclusive<std::uint32_t>::from_json(json["rolls"])) {
        this->parent = parent;
        int index = 0;
        for (auto &entry : json["entries"]) {
            this->children.push_back(new LootEntry((LootTreeNode *)this, entry));
            for (int w = 0; w < entry.contains("weight") ? (int)entry["weight"] : 1; w++) {
                this->entry_lookup.push_back(index);
            }
            index++;
        }   
    }
}
