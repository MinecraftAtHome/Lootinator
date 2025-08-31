#include <cassert>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "lootinator/constraint/constraint.h"


namespace loot {
    // -------------------------------------------------------------------------------
    // ItemAttribute

    bool loot::ItemAttribute::operator==(const ItemAttribute& other) const {
        return type == other.type && level_range == other.level_range;
    }

    bool loot::ItemAttribute::operator!=(const ItemAttribute& other) const {
        return !(*this == other);
    }

    std::ostream& operator<<(std::ostream& os, const ItemAttribute& attribute) {
        return DebugStruct(os, "ItemAttribute")
            .add("type", attribute.type)
            .add("level_range", attribute.level_range)
            .finish();
    }

    // -------------------------------------------------------------------------------
    // Constraint

    bool attributes_match(const std::vector<ItemAttribute>& first, const std::vector<ItemAttribute>& second) {
        if (first.size() != second.size())
            return false;

        for (const auto& e1 : first) {
            bool found = false;
            for (const auto& e2 : second) {
                if (e1.type == e2.type && e1.level_range == e2.level_range) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }

    bool loot::Constraint::item_equal(const Constraint& other) const {
        if (item != other.item || attributes.size() != other.attributes.size()) 
            return false;

        // all item attributes must match
        return loot::attributes_match(attributes, other.attributes);
    }

    bool loot::Constraint::operator==(const Constraint& other) const {
        return item_equal(other) && other.count_range == count_range && other.slot_id == slot_id;
    }

    bool loot::Constraint::operator!=(const Constraint& other) const {
        return !(*this == other);
    }

    std::ostream& operator<<(std::ostream& os, const Constraint& constraint) {
        return DebugStruct(os, "Constraint")
            .add("item", constraint.item)
            .add("count_range", constraint.count_range)
            .add("slot_id", constraint.slot_id)
            .add("attributes", constraint.attributes)
            .finish();
    }

    // -------------------------------------------------------------------------------

    static void merge_into(std::vector<loot::Constraint>& dest, const loot::Constraint& constraint) {
        for (auto& stored_constraint : dest) {
            if (stored_constraint.item_equal(constraint)) {
                // have two item count ranges (min1, max1), (min2, max2)
                // the new min is min1+min2, new max is max1+max2
                stored_constraint.count_range = stored_constraint.count_range.merge(constraint.count_range);
                stored_constraint.slot_id = loot::SLOT_NONE;
                return;
            }
        }

        // did not find the item, create new constraint in the destination vector
        loot::Constraint to_add = constraint;
        to_add.slot_id = loot::SLOT_NONE;
        dest.push_back(to_add);
    }

    // accumulates the per-slot constraints into per-item-type ones (used by seedfinding kernels)
    // the acculumation takes into accout item enchantments
    void merge_contraints(const std::vector<loot::Constraint>& src, std::vector<loot::Constraint>& dest) {
        for (auto& constraint : dest) {
            constraint.slot_id = loot::SLOT_NONE;
        }
        for (const auto& constraint : src) {
            merge_into(dest, constraint);
        }
    }

    std::vector<loot::ItemAttribute> parse_attribute_json(nlohmann::json attribute_json) {
        std::vector<loot::ItemAttribute> attributes;
        for (auto json : attribute_json) {
            attributes.push_back(ItemAttribute::from_json(json));
        }
        return attributes;
    }

    std::vector<loot::Constraint> parse_constraints_from_json(const char *filepath) {
        std::vector<loot::Constraint> constraints;
        std::ifstream f(filepath);
        nlohmann::json data = nlohmann::json::parse(f);
        for (auto con : data) {
            std::uint32_t item = con["item"];
            std::int32_t slot_id = con["slot"];
            loot::RangeInclusive<std::uint32_t> count_range = RangeInclusive<std::uint32_t>::from_json(con["range"]); 
            std::vector<loot::ItemAttribute> attributes = parse_attribute_json(con["attributes"]);
            constraints.push_back({item, count_range, slot_id, attributes});
        }
        return constraints;
    }

    // ------------------------------------------------------------------------------------------------------
    // the following section is for handling constraints satisfiable only by specific pools of the loot table

    PoolConstraint::PoolConstraint(const Constraint &constraint) : Constraint(constraint) {}

    // iterates over the entries for each loot pool and find one that matches
    // the stored item index and the stored attributes
    PoolMatchType PoolConstraint::find_matching_loot_pool(const LootTable &loot_table) {
        PoolMatchType pool_match = PoolMatchType::NOT_FOUND;
        int idx = 0;

        const auto& pools = loot_table.data["pools"];
        for (const auto& loot_pool : pools) {
            const auto& entries = loot_table.data["entries"];
            for (const auto& entry : entries) {
                if (entry_item_matches(loot_table, entry) && entry_attributes_match(entry)) {
                    if (pool_match != PoolMatchType::NOT_FOUND) {
                        pool_match = PoolMatchType::MULTI_POOL;
                    }
                    else {
                        pool_match = PoolMatchType::SINGLE_POOL;
                        pool_idx = idx;
                    }
                }
            }
            idx++;
        }

        return pool_match;
    }

    void PoolConstraint::compute_filter_score(const LootTable &loot_table) {
        // calculate item weight reduction
        int w = 0;
        for (int i : loot_table.precomputed_loot[pool_idx]) {
            w += (item == i ? 1 : 0);
        }
        assert(w != 0);

        float weight_score = static_cast<float>(loot_table.total_weights[pool_idx]) / w;
        float attribute_bonus = attributes.empty() ? 1.0f : 10.0f; // FIXME magic number, also probably requires tuning
        float item_rarity = weight_score * attribute_bonus;

        float forward_score = compute_forward_filter_score(loot_table, item_rarity);
        float backward_penalty = compute_backward_filter_penalty(loot_table);

        filter_score = item_rarity * forward_score * backward_penalty;
    }

    float PoolConstraint::compute_forward_filter_score(const LootTable &loot_table, const float item_rarity) const {
        // TODO
        // compute an approximate performance boost the kernel would get due to 
        // multiple rolls of the same item being required.
        return 1.0f;
    }

    float PoolConstraint::compute_backward_filter_penalty(const LootTable &loot_table) const {
        // TODO
        // compute an approximate performance loss the kernel suffers due to
        // checking multiple backward state advancement options
        return 1.0f;
    }

    bool PoolConstraint::entry_item_matches(const LootTable &loot_table, const nlohmann::json& entry) const {
        if (entry["type"] != "minecraft:item") {
            return false;
        }
        int item_idx = loot_table.find_item_name(entry["name"]);
        return item_idx == item;
    }

    bool PoolConstraint::entry_attributes_match(const nlohmann::json& entry) const {
        // FIXME for now this is sufficient but if user wanted to filter e.g. bastion crossbows
        // they would be classified as multi-pool and thus couldn't be used as a pool constraint
        return true; 
    }

    // -------------------------------------------------------------------------------------------------------

    LootTableConstraintList::LootTableConstraintList(LootTable &loot_table) : loot_table(loot_table) {}
    
    bool LootTableConstraintList::initialize_constraints(const std::vector<loot::Constraint> &constraints) {
        for (const auto& con : constraints) {
            loot::PoolConstraint pool_con(con);
            PoolMatchType match_result = pool_con.find_matching_loot_pool(loot_table);
            if (match_result == PoolMatchType::NOT_FOUND) {
                return false; // illegal constraint specified
            }
            else if (match_result == PoolMatchType::SINGLE_POOL) {
                pool_con.compute_filter_score(loot_table);
                per_pool_constraints.push_back(pool_con);
            }
            else {
                global_constraints.push_back(con);
            }
        }

        // sort the per-pool constraint vector in descending order using the computed heuristic score
        std::sort(per_pool_constraints.begin(), per_pool_constraints.end(), 
            [](const PoolConstraint& a, const PoolConstraint& b) {
                return a.filter_score > b.filter_score;
            }
        );
        
        return true;
    }
}
