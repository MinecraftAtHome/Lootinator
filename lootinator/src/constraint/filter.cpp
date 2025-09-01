#include "lootinator/constraint/filter.h"

namespace loot {
    // iterates over the entries for each loot pool and find one that matches
    // the stored item index and the stored attributes
    int LootTableConstraintList::find_matching_loot_pool(const Constraint& constr) const {
        int pool_match = loot::NO_POOL_MATCH;
        int pool_idx = 0;

        const auto& pools = loot_table.data["pools"];
        for (const auto& loot_pool : pools) {
            const auto& entries = loot_table.data["entries"];
            for (const auto& entry : entries) {
                if (entry_item_matches(constr, entry) && entry_attributes_match(constr, entry)) {
                    if (pool_match != loot::NO_POOL_MATCH) {
                        pool_match = loot::MULTI_POOL_MATCH;
                    }
                    else {
                        pool_match = pool_idx;
                    }
                }
            }
            pool_idx++;
        }

        return pool_match;
    }

    void PoolFilter::compute_filter_score(const LootTable &loot_table) {
        // calculate item weight reduction
        nlohmann::json entry = loot_table.data["pools"][pool_idx]["entries"][entry_idx];
        int w = entry.contains("weight") ? entry["weight"] : 1;
        assert(w != 0);
        float weight_score = static_cast<float>(loot_table.total_weights[pool_idx]) / w;

        // calculate other stuff
    }

    float PoolFilter::compute_forward_filter_score(const LootTable &loot_table, const float item_rarity) const {
        // TODO
        // compute an approximate performance boost the kernel would get due to 
        // multiple rolls of the same item being required.
        return 1.0f;
    }

    float PoolFilter::compute_backward_filter_penalty(const LootTable &loot_table) const {
        // TODO
        // compute an approximate performance loss the kernel suffers due to
        // checking multiple backward state advancement options
        return 1.0f;
    }

    bool LootTableConstraintList::entry_item_matches(const Constraint& constr, const nlohmann::json& entry) const {
        if (entry["type"] != "minecraft:item") {
            return false;
        }
        int item_idx = loot_table.find_item_name(entry["name"]);
        return item_idx == constr.item;
    }

    bool LootTableConstraintList::entry_attributes_match(const Constraint& constr, const nlohmann::json& entry) const {
        // FIXME for now this is sufficient but if user wanted to filter e.g. bastion crossbows
        // they would be classified as multi-pool and thus couldn't be used as a pool constraint
        return true; 
    }

    // -------------------------------------------------------------------------------------------------------

    LootTableConstraintList::LootTableConstraintList(LootTable &loot_table) : loot_table(loot_table) {}
    
    bool LootTableConstraintList::initialize_constraints(const std::vector<loot::Constraint> &constraints) {
        for (const auto& constr : constraints) {
            int pool_idx = find_matching_loot_pool(constr);
            if (pool_idx == loot::NO_POOL_MATCH) {
                return false; // illegal constraint specified
            }
            else if (pool_idx == loot::MULTI_POOL_MATCH) {
                global_constraints.push_back(constr);
            }
            else {
                add_possible_filters(constraints, constr);
            }
        }

        // sort the per-pool constraint vector in descending order using the computed heuristic score
        std::sort(available_filters.begin(), available_filters.end(), 
            [](const PoolFilter& a, const PoolFilter& b) {
                return a.filter_score > b.filter_score;
            }
        );
        
        return true;
    }

    void add_possible_filters(const std::vector<loot::Constraint>& constraints, const Constraint& main_constraint) {
        // TODO
        // this will add unique filters to the possible filter list
    }
}