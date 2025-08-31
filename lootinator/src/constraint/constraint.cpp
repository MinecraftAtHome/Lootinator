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

    PoolMatchType PoolConstraint::find_matching_loot_pool(LootTable &loot_table) {
        // TODO
        return PoolMatchType();
    }

    void PoolConstraint::compute_filter_score(LootTable &loot_table) {
        // TODO
    }

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
